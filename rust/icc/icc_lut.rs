// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use crate::ffi;
use crate::icc_curve::{self, EmptyCurve};
use crate::icc_reader::{align_to_4, Reader};

const MFT1_SIGNATURE: u32 = u32::from_be_bytes(*b"mft1");
const MFT2_SIGNATURE: u32 = u32::from_be_bytes(*b"mft2");
const MAB_SIGNATURE: u32 = u32::from_be_bytes(*b"mAB ");
const MBA_SIGNATURE: u32 = u32::from_be_bytes(*b"mBA ");
const MAX_CLUT_ELEMENTS: usize = 10_000_000;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) enum Direction {
    A2B,
    B2A,
}

pub(crate) enum ParsedLut {
    A2B(ffi::A2B),
    B2A(ffi::B2A),
}

pub(crate) fn parse_lut(data: &[u8], pcs_is_xyz: bool, direction: Direction) -> Option<ParsedLut> {
    let reader = Reader::new(data);
    let a2b = match reader.be_u32(0)? {
        MFT1_SIGNATURE => parse_legacy(reader, 1, pcs_is_xyz, direction)?,
        MFT2_SIGNATURE => parse_legacy(reader, 2, pcs_is_xyz, direction)?,
        MAB_SIGNATURE if direction == Direction::A2B => {
            parse_modern(reader, pcs_is_xyz, direction)?
        }
        MBA_SIGNATURE if direction == Direction::B2A => {
            parse_modern(reader, pcs_is_xyz, direction)?
        }
        _ => return None,
    };

    Some(match direction {
        Direction::A2B => ParsedLut::A2B(a2b),
        Direction::B2A => ParsedLut::B2A(a2b_to_b2a(a2b)),
    })
}

fn parse_legacy(
    reader: Reader<'_>,
    byte_width: usize,
    pcs_is_xyz: bool,
    direction: Direction,
) -> Option<ffi::A2B> {
    let header_size = if byte_width == 1 { 48 } else { 52 };
    reader.bytes(0, header_size)?;

    let input_channels = reader.u8(8)? as usize;
    let output_channels = reader.u8(9)? as usize;
    if !(1..=4).contains(&input_channels) || output_channels != 3 {
        return None;
    }

    let grid_size = reader.u8(10)?;
    if grid_size < 2 {
        return None;
    }

    let input_entries = if byte_width == 1 {
        256
    } else {
        reader.be_u16(48)? as usize
    };
    let output_entries = if byte_width == 1 {
        256
    } else {
        reader.be_u16(50)? as usize
    };
    if !(2..=4096).contains(&input_entries) || !(2..=4096).contains(&output_entries)
    {
        return None;
    }

    let input_values = input_entries.checked_mul(input_channels)?;
    let input_bytes = input_values.checked_mul(byte_width)?;
    let input_data = reader.bytes(header_size, input_bytes)?;

    let grid_points_count = checked_pow(grid_size as usize, input_channels)?;
    if grid_points_count > MAX_CLUT_ELEMENTS {
        return None;
    }
    let clut_values = grid_points_count.checked_mul(output_channels)?;
    if clut_values > MAX_CLUT_ELEMENTS {
        return None;
    }
    let clut_bytes = clut_values.checked_mul(byte_width)?;
    let clut_offset = header_size.checked_add(input_bytes)?;
    let clut_data = reader.bytes(clut_offset, clut_bytes)?;

    let output_values = output_entries.checked_mul(output_channels)?;
    let output_bytes = output_values.checked_mul(byte_width)?;
    let output_offset = clut_offset.checked_add(clut_bytes)?;
    let output_data = reader.bytes(output_offset, output_bytes)?;

    let mut matrix = parse_matrix(reader, 12)?;
    let mut matrix_bias = [0.0; 3];
    apply_encoding_factor(&mut matrix, &mut matrix_bias, pcs_is_xyz, direction);

    let input_curves = split_legacy_curves(input_data, input_entries, input_channels, byte_width)?;
    let output_curves =
        split_legacy_curves(output_data, output_entries, output_channels, byte_width)?;
    let (grid_data, is_16bit_grid) = copy_grid_data(clut_data, byte_width)?;

    let mut grid_points = [0; 4];
    grid_points[..input_channels].fill(grid_size);

    Some(ffi::A2B {
        input_curves,
        input_channels: input_channels as u32,
        grid_points,
        grid_data,
        is_16bit_grid,
        matrix_curves: Vec::new(),
        matrix,
        matrix_bias,
        matrix_channels: 0,
        output_curves,
        output_channels: output_channels as u32,
    })
}

fn parse_modern(
    reader: Reader<'_>,
    pcs_is_xyz: bool,
    direction: Direction,
) -> Option<ffi::A2B> {
    reader.bytes(0, 48)?;
    let input_channels = reader.u8(8)? as usize;
    let output_channels = reader.u8(9)? as usize;
    if input_channels > 4 || output_channels != 3 {
        return None;
    }

    let b_offset = reader.be_u32(12)? as usize;
    let matrix_offset = reader.be_u32(16)? as usize;
    let m_offset = reader.be_u32(20)? as usize;
    let clut_offset = reader.be_u32(24)? as usize;
    let a_offset = reader.be_u32(28)? as usize;

    if (matrix_offset == 0) != (m_offset == 0) || (clut_offset == 0) != (a_offset == 0) {
        return None;
    }

    let b_count = match direction {
        Direction::A2B => output_channels,
        Direction::B2A => input_channels,
    };
    let m_count = b_count;
    let a_count = match direction {
        Direction::A2B => input_channels,
        Direction::B2A => output_channels,
    };

    if b_offset == 0 {
        return None;
    }
    let output_curves = parse_curve_sequence(reader, b_offset, b_count)?;
    if output_curves.is_empty() {
        return None;
    }

    let matrix_curves = if m_offset == 0 {
        Vec::new()
    } else {
        parse_curve_sequence(reader, m_offset, m_count)?
    };
    let matrix_channels = if matrix_curves.is_empty() { 0 } else { 3 };

    let (mut matrix, mut matrix_bias) = if matrix_offset == 0 {
        (identity_matrix(), [0.0; 3])
    } else {
        (
            parse_matrix(reader, matrix_offset)?,
            [
                reader.s15_fixed16(matrix_offset + 36)?,
                reader.s15_fixed16(matrix_offset + 40)?,
                reader.s15_fixed16(matrix_offset + 44)?,
            ],
        )
    };
    apply_encoding_factor(&mut matrix, &mut matrix_bias, pcs_is_xyz, direction);

    let (input_curves, final_input_channels, grid_points, grid_data, is_16bit_grid) =
        if clut_offset == 0 {
            if input_channels != output_channels {
                return None;
            }
            (Vec::new(), 0, [0; 4], Vec::new(), false)
        } else {
            if a_offset == 0 || input_channels == 0 {
                return None;
            }
            let input_curves = parse_curve_sequence(reader, a_offset, a_count)?;
            if input_curves.is_empty() {
                return None;
            }
            let (grid_points, grid_data, is_16bit_grid) =
                parse_modern_clut(reader, clut_offset, input_channels, output_channels)?;
            (
                input_curves,
                input_channels as u32,
                grid_points,
                grid_data,
                is_16bit_grid,
            )
        };

    Some(ffi::A2B {
        input_curves,
        input_channels: final_input_channels,
        grid_points,
        grid_data,
        is_16bit_grid,
        matrix_curves,
        matrix,
        matrix_bias,
        matrix_channels,
        output_curves,
        output_channels: output_channels as u32,
    })
}

fn parse_curve_sequence(reader: Reader<'_>, mut offset: usize, count: usize) -> Option<Vec<ffi::Curve>> {
    let mut curves = Vec::with_capacity(count);
    for _ in 0..count {
        let data = reader.bytes(offset, reader.len().checked_sub(offset)?)?;
        let parsed = icc_curve::parse_curve(data, EmptyCurve::Identity)?;
        offset = align_to_4(offset.checked_add(parsed.bytes_used)?)?;
        curves.push(parsed.curve);
    }
    Some(curves)
}

fn parse_modern_clut(
    reader: Reader<'_>,
    offset: usize,
    input_channels: usize,
    output_channels: usize,
) -> Option<([u8; 4], Vec<u8>, bool)> {
    reader.bytes(offset, 20)?;
    let byte_width = reader.u8(offset + 16)? as usize;
    if byte_width != 1 && byte_width != 2 {
        return None;
    }

    let mut grid_points = [0; 4];
    let mut grid_points_count = 1usize;
    for (axis, grid_point) in grid_points[..input_channels].iter_mut().enumerate() {
        *grid_point = reader.u8(offset + axis)?;
        if *grid_point < 2 {
            return None;
        }
        grid_points_count = grid_points_count.checked_mul(*grid_point as usize)?;
    }
    let clut_values = grid_points_count.checked_mul(output_channels)?;
    if clut_values > MAX_CLUT_ELEMENTS {
        return None;
    }
    let clut_bytes = clut_values.checked_mul(byte_width)?;
    let data = reader.bytes(offset.checked_add(20)?, clut_bytes)?;
    let (grid_data, is_16bit_grid) = copy_grid_data(data, byte_width)?;
    Some((grid_points, grid_data, is_16bit_grid))
}

fn parse_matrix(reader: Reader<'_>, offset: usize) -> Option<ffi::Matrix3x3> {
    reader.bytes(offset, 36)?;
    let mut vals = [[0.0; 3]; 3];
    for (row, values) in vals.iter_mut().enumerate() {
        for (column, value) in values.iter_mut().enumerate() {
            *value = reader.s15_fixed16(offset + (row * 3 + column) * 4)?;
        }
    }
    Some(ffi::Matrix3x3 { vals })
}

fn identity_matrix() -> ffi::Matrix3x3 {
    ffi::Matrix3x3 {
        vals: [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]],
    }
}

fn apply_encoding_factor(
    matrix: &mut ffi::Matrix3x3,
    matrix_bias: &mut [f32; 3],
    pcs_is_xyz: bool,
    direction: Direction,
) {
    if !pcs_is_xyz {
        return;
    }
    let factor = match direction {
        Direction::A2B => 65535.0 / 32768.0,
        Direction::B2A => 32768.0 / 65535.0,
    };
    for (row, bias) in matrix_bias.iter_mut().enumerate() {
        for value in &mut matrix.vals[row] {
            *value *= factor;
        }
        *bias *= factor;
    }
}

fn split_legacy_curves(
    data: &[u8],
    entries: usize,
    channels: usize,
    byte_width: usize,
) -> Option<Vec<ffi::Curve>> {
    let bytes_per_curve = entries.checked_mul(byte_width)?;
    if data.len() != bytes_per_curve.checked_mul(channels)? {
        return None;
    }

    let mut curves = Vec::with_capacity(channels);
    for channel in 0..channels {
        let start = channel.checked_mul(bytes_per_curve)?;
        let table = data.get(start..start + bytes_per_curve)?;
        let table_data = if byte_width == 1 {
            let mut expanded = Vec::with_capacity(entries * 2);
            for value in table {
                expanded.extend([*value, *value]);
            }
            expanded
        } else {
            table.to_vec()
        };
        curves.push(ffi::Curve {
            table_entries: entries as u32,
            parametric: ffi::TransferFunction::default(),
            table_data,
        });
    }
    Some(curves)
}

fn copy_grid_data(data: &[u8], byte_width: usize) -> Option<(Vec<u8>, bool)> {
    let is_16bit_grid = byte_width == 2;
    let overread = if is_16bit_grid { 2 } else { 1 };
    let padded_len = align_to_4(data.len().checked_add(overread)?)?;
    let mut grid_data = Vec::with_capacity(padded_len);
    grid_data.extend_from_slice(data);
    grid_data.resize(padded_len, 0);
    Some((grid_data, is_16bit_grid))
}

fn checked_pow(base: usize, exponent: usize) -> Option<usize> {
    let mut result = 1usize;
    for _ in 0..exponent {
        result = result.checked_mul(base)?;
    }
    Some(result)
}

fn a2b_to_b2a(a2b: ffi::A2B) -> ffi::B2A {
    ffi::B2A {
        input_curves: a2b.output_curves,
        input_channels: a2b.output_channels,
        matrix: a2b.matrix,
        matrix_bias: a2b.matrix_bias,
        matrix_curves: a2b.matrix_curves,
        matrix_channels: a2b.matrix_channels,
        grid_points: a2b.grid_points,
        grid_data: a2b.grid_data,
        is_16bit_grid: a2b.is_16bit_grid,
        output_curves: a2b.input_curves,
        output_channels: a2b.input_channels,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn fixed(value: f32) -> [u8; 4] {
        ((value * 65536.0) as i32).to_be_bytes()
    }

    fn legacy_lut(byte_width: usize, input_channels: u8) -> Vec<u8> {
        let input_entries = if byte_width == 1 { 256 } else { 2 };
        let output_entries = if byte_width == 1 { 256 } else { 2 };
        let header_size = if byte_width == 1 { 48 } else { 52 };
        let mut data = vec![0; header_size];
        data[0..4].copy_from_slice(if byte_width == 1 { b"mft1" } else { b"mft2" });
        data[8] = input_channels;
        data[9] = 3;
        data[10] = 2;
        for index in 0..3 {
            data[12 + (index * 3 + index) * 4..16 + (index * 3 + index) * 4]
                .copy_from_slice(&fixed(1.0));
        }
        if byte_width == 2 {
            data[48..50].copy_from_slice(&(input_entries as u16).to_be_bytes());
            data[50..52].copy_from_slice(&(output_entries as u16).to_be_bytes());
        }

        let input_values = input_entries * input_channels as usize;
        let clut_values = 2usize.pow(input_channels as u32) * 3;
        let output_values = output_entries * 3;
        for index in 0..input_values + clut_values + output_values {
            if byte_width == 1 {
                data.push(index as u8);
            } else {
                data.extend((index as u16).to_be_bytes());
            }
        }
        data
    }

    fn identity_curve() -> [u8; 12] {
        *b"curv\0\0\0\0\0\0\0\0"
    }

    fn modern_lut(signature: &[u8; 4], with_clut: bool) -> Vec<u8> {
        let mut data = vec![0; 32];
        data[0..4].copy_from_slice(signature);
        data[8] = 3;
        data[9] = 3;

        data[12..16].copy_from_slice(&32u32.to_be_bytes());
        for _ in 0..3 {
            data.extend(identity_curve());
        }

        if with_clut {
            data[28..32].copy_from_slice(&68u32.to_be_bytes());
            for _ in 0..3 {
                data.extend(identity_curve());
            }

            data[24..28].copy_from_slice(&104u32.to_be_bytes());
            let mut clut_header = [0; 20];
            clut_header[0..3].fill(2);
            clut_header[16] = 1;
            data.extend(clut_header);
            data.extend(0..24);
        }
        data
    }

    #[test]
    fn parses_legacy_lut8_and_expands_curves() {
        let ParsedLut::A2B(a2b) = parse_lut(&legacy_lut(1, 3), false, Direction::A2B).unwrap()
        else {
            panic!("expected A2B");
        };
        assert_eq!(a2b.input_channels, 3);
        assert_eq!(a2b.output_channels, 3);
        assert_eq!(a2b.grid_points, [2, 2, 2, 0]);
        assert_eq!(a2b.input_curves.len(), 3);
        assert_eq!(a2b.input_curves[0].table_entries, 256);
        assert_eq!(a2b.input_curves[0].table_data[0..4], [0, 0, 1, 1]);
        assert!(!a2b.is_16bit_grid);
        assert_eq!(a2b.grid_data.len() % 4, 0);
    }

    #[test]
    fn parses_legacy_lut16_and_reverses_b2a() {
        let ParsedLut::B2A(b2a) = parse_lut(&legacy_lut(2, 4), false, Direction::B2A).unwrap()
        else {
            panic!("expected B2A");
        };
        assert_eq!(b2a.input_channels, 3);
        assert_eq!(b2a.output_channels, 4);
        assert_eq!(b2a.input_curves.len(), 3);
        assert_eq!(b2a.output_curves.len(), 4);
        assert_eq!(b2a.grid_points, [2, 2, 2, 2]);
        assert!(b2a.is_16bit_grid);
        assert_eq!(b2a.grid_data.len() % 4, 0);
    }

    #[test]
    fn parses_modern_lut_without_optional_clut() {
        let ParsedLut::A2B(a2b) =
            parse_lut(&modern_lut(b"mAB ", false), false, Direction::A2B).unwrap()
        else {
            panic!("expected A2B");
        };
        assert_eq!(a2b.input_channels, 0);
        assert!(a2b.input_curves.is_empty());
        assert_eq!(a2b.output_curves.len(), 3);
        assert_eq!(a2b.output_curves[0].parametric.g, 1.0);
        assert_eq!(a2b.output_channels, 3);
    }

    #[test]
    fn parses_modern_clut_and_reverses_b2a() {
        let ParsedLut::B2A(b2a) =
            parse_lut(&modern_lut(b"mBA ", true), false, Direction::B2A).unwrap()
        else {
            panic!("expected B2A");
        };
        assert_eq!(b2a.input_channels, 3);
        assert_eq!(b2a.output_channels, 3);
        assert_eq!(b2a.input_curves.len(), 3);
        assert_eq!(b2a.output_curves.len(), 3);
        assert_eq!(b2a.grid_points, [2, 2, 2, 0]);
        assert_eq!(&b2a.grid_data[..24], &(0..24).collect::<Vec<_>>());
        assert_eq!(b2a.grid_data.len() % 4, 0);
    }

    #[test]
    fn rejects_legacy_lut_overflows_and_invalid_channels() {
        let mut zero_grid = legacy_lut(2, 3);
        zero_grid[10] = 0;
        assert!(parse_lut(&zero_grid, false, Direction::A2B).is_none());

        let mut too_many_channels = legacy_lut(2, 3);
        too_many_channels[8] = 5;
        assert!(parse_lut(&too_many_channels, false, Direction::A2B).is_none());

        let mut wrong_outputs = legacy_lut(2, 3);
        wrong_outputs[9] = 4;
        assert!(parse_lut(&wrong_outputs, false, Direction::A2B).is_none());

        let mut truncated = legacy_lut(2, 3);
        truncated.pop();
        assert!(parse_lut(&truncated, false, Direction::A2B).is_none());
    }

    #[test]
    fn rejects_unpaired_modern_lut_stages() {
        let mut matrix_without_curves = modern_lut(b"mAB ", false);
        matrix_without_curves[16..20].copy_from_slice(&68u32.to_be_bytes());
        matrix_without_curves.resize(116, 0);
        assert!(parse_lut(&matrix_without_curves, false, Direction::A2B).is_none());

        let mut curves_without_matrix = modern_lut(b"mAB ", false);
        curves_without_matrix[20..24].copy_from_slice(&68u32.to_be_bytes());
        curves_without_matrix.resize(104, 0);
        assert!(parse_lut(&curves_without_matrix, false, Direction::A2B).is_none());

        let mut clut_without_curves = modern_lut(b"mAB ", false);
        clut_without_curves[24..28].copy_from_slice(&68u32.to_be_bytes());
        clut_without_curves.resize(112, 0);
        assert!(parse_lut(&clut_without_curves, false, Direction::A2B).is_none());

        let mut curves_without_clut = modern_lut(b"mAB ", false);
        curves_without_clut[28..32].copy_from_slice(&68u32.to_be_bytes());
        curves_without_clut.resize(104, 0);
        assert!(parse_lut(&curves_without_clut, false, Direction::A2B).is_none());
    }
}
