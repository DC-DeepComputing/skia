// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser FFI bindings.
//!
//! Provides a pure Rust ICC parser that mirrors the parsing logic, organization,
//! and constants of skcms. All parsing happens in Rust for memory safety,
//! populating data structures for use by skcms / Skia codecs.

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    extern "C++" {
        include!("modules/skcms/skcms.h");
        include!("rust/icc/FFI.h");
    }

    /// 3x3 matrix for color space transforms (matches `skcms_Matrix3x3`).
    #[derive(Clone, Copy)]
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }

    /// Transfer function parameters (matches `skcms_TransferFunction`).
    #[derive(Clone, Copy, Default)]
    struct TransferFunction {
        g: f32,
        a: f32,
        b: f32,
        c: f32,
        d: f32,
        e: f32,
        f: f32,
    }

    /// CICP color metadata (matches `skcms_CICP` and `CICP_Layout`).
    #[derive(Clone, Copy)]
    struct Cicp {
        color_primaries: u8,
        transfer_characteristics: u8,
        matrix_coefficients: u8,
        video_full_range_flag: u8,
    }

    /// Curve for LUT transforms - parametric or table-based (matches `skcms_Curve`).
    #[derive(Clone)]
    struct Curve {
        table_entries: u32, // 0 = parametric, >0 = table
        parametric: TransferFunction,
        table_data: Vec<u8>, // u16 values as bytes (big-endian) for tables
    }

    /// Multi-stage device <-> PCS transform (matches `skcms_A2B` and `skcms_B2A`).
    struct A2BOrB2ATransform {
        input_curves: Vec<Curve>,
        input_channels: u32,
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        is_16bit_grid: bool,
        matrix_curves: Vec<Curve>,
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_channels: u32,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    /// Parsed ICC profile data (matches `skcms_ICCProfile`).
    struct IccProfile {
        data_color_space: u32,
        connection_space: u32,
        to_xyzd50: Matrix3x3,
        has_to_xyzd50: bool,
        /// Transfer curves for R, G, B channels (or gray replicated x3).
        trc_r: Curve,
        trc_g: Curve,
        trc_b: Curve,
        has_trc: bool,
        cicp: Cicp,
        has_cicp: bool,
        a2b: A2BOrB2ATransform,
        has_a2b: bool,
        b2a: A2BOrB2ATransform,
        has_b2a: bool,
    }

    extern "Rust" {
        /// Parses ICC profile from `data`. If successful, returns `true`
        /// and writes result to `out`. If failure, returns `false`.
        /// (Matches `skcms_ParseWithA2BPriority`).
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}

// =============================================================================
// Additional ICC signature values that are only used internally
// =============================================================================

// File signature
const SKCMS_SIGNATURE_ACSP: u32 = u32::from_be_bytes(*b"acsp");

// Tag signatures
const SKCMS_SIGNATURE_R_TRC: u32 = u32::from_be_bytes(*b"rTRC");
const SKCMS_SIGNATURE_G_TRC: u32 = u32::from_be_bytes(*b"gTRC");
const SKCMS_SIGNATURE_B_TRC: u32 = u32::from_be_bytes(*b"bTRC");
const SKCMS_SIGNATURE_K_TRC: u32 = u32::from_be_bytes(*b"kTRC");

const SKCMS_SIGNATURE_R_XYZ: u32 = u32::from_be_bytes(*b"rXYZ");
const SKCMS_SIGNATURE_G_XYZ: u32 = u32::from_be_bytes(*b"gXYZ");
const SKCMS_SIGNATURE_B_XYZ: u32 = u32::from_be_bytes(*b"bXYZ");

const SKCMS_SIGNATURE_A2B0: u32 = u32::from_be_bytes(*b"A2B0");
const SKCMS_SIGNATURE_A2B1: u32 = u32::from_be_bytes(*b"A2B1");

const SKCMS_SIGNATURE_B2A0: u32 = u32::from_be_bytes(*b"B2A0");
const SKCMS_SIGNATURE_B2A1: u32 = u32::from_be_bytes(*b"B2A1");

const SKCMS_SIGNATURE_CICP: u32 = u32::from_be_bytes(*b"cicp");

// Type signatures
const SKCMS_SIGNATURE_CURV: u32 = u32::from_be_bytes(*b"curv");
const SKCMS_SIGNATURE_MFT1: u32 = u32::from_be_bytes(*b"mft1");
const SKCMS_SIGNATURE_MFT2: u32 = u32::from_be_bytes(*b"mft2");
const SKCMS_SIGNATURE_M_AB: u32 = u32::from_be_bytes(*b"mAB ");
const SKCMS_SIGNATURE_M_BA: u32 = u32::from_be_bytes(*b"mBA ");
const SKCMS_SIGNATURE_PARA: u32 = u32::from_be_bytes(*b"para");

// Color space and PCS signatures
const SKCMS_SIGNATURE_XYZ: u32 = u32::from_be_bytes(*b"XYZ ");
const SKCMS_SIGNATURE_LAB: u32 = u32::from_be_bytes(*b"Lab ");
const SKCMS_SIGNATURE_GRAY: u32 = u32::from_be_bytes(*b"GRAY");

// Parametric curve function types (enum { kG = 0, kGAB = 1, kGABC = 2, kGABCD = 3, kGABCDEF = 4 };)
const K_G: u16 = 0;
const K_GAB: u16 = 1;
const K_GABC: u16 = 2;
const K_GABCD: u16 = 3;
const K_GABCDEF: u16 = 4;

// Limits (mirroring header_Layout, tag_Layout, SKCMS_MAX_GRID_POINTS, kMaxTableEntries)
// Maps to header_Layout: SAFE_SIZEOF(header_Layout) == 132 (including tag_count)
const SAFE_HEADER_SIZE: usize = 132;
// Maps to tag_Layout: SAFE_SIZEOF(tag_Layout) == 12
const SAFE_TAG_ENTRY_SIZE: usize = 12;
// SKCMS_MAX_GRID_POINTS
const SKCMS_MAX_GRID_POINTS: u64 = 350_000_000;
// See https://crbug.com/492744328 for how this was determined (kMaxTableEntries).
const K_MAX_TABLE_ENTRIES: usize = 1 << 24; // 16,777,216

// =============================================================================
// Helper Reading Primitives
// =============================================================================

/// Reads a 16-bit big-endian unsigned integer (matches `read_big_u16`).
#[inline]
fn read_u16(buf: &[u8]) -> Option<u16> {
    let bytes: [u8; 2] = buf.get(..2)?.try_into().ok()?;
    Some(u16::from_be_bytes(bytes))
}

/// Reads a 32-bit big-endian unsigned integer (matches `read_big_u32`).
#[inline]
fn read_u32(buf: &[u8]) -> Option<u32> {
    let bytes: [u8; 4] = buf.get(..4)?.try_into().ok()?;
    Some(u32::from_be_bytes(bytes))
}

/// Reads a 32-bit big-endian signed integer (matches `read_big_i32`).
#[inline]
fn read_i32(buf: &[u8]) -> Option<i32> {
    let bytes: [u8; 4] = buf.get(..4)?.try_into().ok()?;
    Some(i32::from_be_bytes(bytes))
}

/// Reads an s15.16 fixed-point big-endian float (matches `read_big_fixed`).
#[inline]
fn read_fixed(buf: &[u8]) -> Option<f32> {
    let raw = read_i32(buf)?;
    Some((raw as f32) * (1.0 / 65536.0))
}

/// Represents an ICC tag entry in the profile (matches `skcms_ICCTag` and `tag_Layout`).
struct IccTag<'a> {
    offset: usize,
    tag_type: u32,
    buf: &'a [u8],
}

/// Looks up an ICC tag by its signature (matches `skcms_GetTagBySignature`).
fn get_tag_by_signature<'a>(data: &'a [u8], tag_count: usize, sig: u32) -> Option<IccTag<'a>> {
    let tag_table = data.get(SAFE_HEADER_SIZE..)?;
    for i in 0..tag_count {
        let entry = tag_table.get(i * SAFE_TAG_ENTRY_SIZE..(i + 1) * SAFE_TAG_ENTRY_SIZE)?;
        let tag_sig = read_u32(&entry[0..4])?;
        if tag_sig == sig {
            let offset = read_u32(&entry[4..8])? as usize;
            let size = read_u32(&entry[8..12])? as usize;
            let tag_buf = data.get(offset..offset.checked_add(size)?)?;
            let tag_type = read_u32(tag_buf)?;
            return Some(IccTag {
                offset,
                tag_type,
                buf: tag_buf,
            });
        }
    }
    None
}

// =============================================================================
// XYZ and Colorant Matrix Parsing
// =============================================================================

// XYZType is technically variable sized, holding N XYZ triples. However, the only valid uses of
// the type are for tags/data that store exactly one triple.
// Matches `XYZ_Layout` and `read_tag_xyz`.
fn read_tag_xyz(tag: &IccTag) -> Option<[f32; 3]> {
    if tag.tag_type != SKCMS_SIGNATURE_XYZ || tag.buf.len() < 20 {
        return None;
    }
    Some([
        read_fixed(&tag.buf[8..12])?,
        read_fixed(&tag.buf[12..16])?,
        read_fixed(&tag.buf[16..20])?,
    ])
}

/// Reads rXYZ, gXYZ, bXYZ tags into a 3x3 toXYZD50 matrix (matches `read_to_XYZD50`).
fn read_to_xyzd50(r_tag: &IccTag, g_tag: &IccTag, b_tag: &IccTag) -> Option<ffi::Matrix3x3> {
    let r = read_tag_xyz(r_tag)?;
    let g = read_tag_xyz(g_tag)?;
    let b = read_tag_xyz(b_tag)?;
    Some(ffi::Matrix3x3 {
        vals: [
            [r[0], g[0], b[0]],
            [r[1], g[1], b[1]],
            [r[2], g[2], b[2]],
        ],
    })
}

// =============================================================================
// Curve Parsing ('para' and 'curv')
// =============================================================================

/// Basic soundness checks for sRGBish transfer functions (matches `skcms_TransferFunction_isSRGBish`).
fn is_srgb_ish(tf: &ffi::TransferFunction) -> bool {
    let sum = tf.a + tf.b + tf.c + tf.d + tf.e + tf.f + tf.g;
    // a,c,d,g should be non-negative to make any sense.
    // Raising a negative value to a fractional tf->g produces complex numbers.
    sum.is_finite()
        && tf.a >= 0.0
        && tf.c >= 0.0
        && tf.d >= 0.0
        && tf.g >= 0.0
        && (tf.a * tf.d + tf.b) >= 0.0
}

/// Parses parametric curve data (matches `read_curve_para` and `para_Layout`).
/// 1, 3, 4, 5, or 7 s15.16, depending on function_type.
fn read_curve_para(buf: &[u8]) -> Option<(ffi::Curve, usize)> {
    if buf.len() < 12 {
        return None;
    }

    let function_type = read_u16(&buf[8..10])?;
    if function_type > K_GABCDEF {
        return None;
    }

    const CURVE_BYTES: [usize; 5] = [4, 12, 16, 20, 28];
    let bytes_needed = CURVE_BYTES[function_type as usize];
    if buf.len() < 12 + bytes_needed {
        return None;
    }

    let var = &buf[12..];
    let mut tf = ffi::TransferFunction {
        a: 1.0,
        b: 0.0,
        c: 0.0,
        d: 0.0,
        e: 0.0,
        f: 0.0,
        g: read_fixed(&var[0..4])?,
    };

    switch_para_function(function_type, var, &mut tf)?;

    if !is_srgb_ish(&tf) {
        return None;
    }

    Some((
        ffi::Curve {
            table_entries: 0,
            parametric: tf,
            table_data: Vec::new(),
        },
        12 + bytes_needed,
    ))
}

#[inline]
fn switch_para_function(function_type: u16, var: &[u8], tf: &mut ffi::TransferFunction) -> Option<()> {
    match function_type {
        K_G => {}
        K_GAB => {
            tf.a = read_fixed(&var[4..8])?;
            tf.b = read_fixed(&var[8..12])?;
            if tf.a == 0.0 {
                return None;
            }
            tf.d = -tf.b / tf.a;
        }
        K_GABC => {
            tf.a = read_fixed(&var[4..8])?;
            tf.b = read_fixed(&var[8..12])?;
            tf.e = read_fixed(&var[12..16])?;
            if tf.a == 0.0 {
                return None;
            }
            tf.d = -tf.b / tf.a;
            tf.f = tf.e;
        }
        K_GABCD => {
            tf.a = read_fixed(&var[4..8])?;
            tf.b = read_fixed(&var[8..12])?;
            tf.c = read_fixed(&var[12..16])?;
            tf.d = read_fixed(&var[16..20])?;
        }
        K_GABCDEF => {
            tf.a = read_fixed(&var[4..8])?;
            tf.b = read_fixed(&var[8..12])?;
            tf.c = read_fixed(&var[12..16])?;
            tf.d = read_fixed(&var[16..20])?;
            tf.e = read_fixed(&var[20..24])?;
            tf.f = read_fixed(&var[24..28])?;
        }
        _ => return None,
    }
    Some(())
}

/// Parses table or simple gamma curve data (matches `read_curve_curv` and `curv_Layout`).
/// value_count, 8.8 if 1, uint16 (n*65535) if > 1.
fn read_curve_curv(buf: &[u8]) -> Option<(ffi::Curve, usize)> {
    if buf.len() < 12 {
        return None;
    }

    let value_count = read_u32(&buf[8..12])? as usize;
    let total_size = 12usize.checked_add(value_count.checked_mul(2)?)?;
    if buf.len() < total_size {
        return None;
    }

    if value_count < 2 {
        let g = if value_count == 0 {
            // Empty tables are a shorthand for an identity curve
            1.0
        } else {
            // Single entry tables are a shorthand for simple gamma
            (read_u16(&buf[12..14])? as f32) * (1.0 / 256.0)
        };
        return Some((
            ffi::Curve {
                table_entries: 0,
                parametric: ffi::TransferFunction {
                    g,
                    a: 1.0,
                    b: 0.0,
                    c: 0.0,
                    d: 0.0,
                    e: 0.0,
                    f: 0.0,
                },
                table_data: Vec::new(),
            },
            total_size,
        ));
    }

    if value_count > K_MAX_TABLE_ENTRIES {
        return None;
    }

    Some((
        ffi::Curve {
            table_entries: value_count as u32,
            parametric: ffi::TransferFunction::default(),
            table_data: buf[12..total_size].to_vec(),
        },
        total_size,
    ))
}

// Parses both curveType and parametricCurveType data. Ensures that at most 'size' bytes are read.
// (Matches `read_curve`).
fn read_curve(buf: &[u8]) -> Option<(ffi::Curve, usize)> {
    if buf.len() < 4 {
        return None;
    }
    let tag_type = read_u32(&buf[0..4])?;
    if tag_type == SKCMS_SIGNATURE_PARA {
        read_curve_para(buf)
    } else if tag_type == SKCMS_SIGNATURE_CURV {
        read_curve_curv(buf)
    } else {
        None
    }
}

/// Reads a sequence of N curves, taking 4-byte padding into account (matches `read_curves`).
fn read_curves(buf: &[u8], mut offset: usize, num_curves: usize) -> Option<Vec<ffi::Curve>> {
    let mut curves = Vec::with_capacity(num_curves);
    for _ in 0..num_curves {
        if offset > buf.len() {
            return None;
        }
        let (curve, curve_bytes) = read_curve(buf.get(offset..)?)?;
        curves.push(curve);
        let padded_bytes = (curve_bytes.checked_add(3)?) & !3;
        offset = offset.checked_add(padded_bytes)?;
    }
    Some(curves)
}

// =============================================================================
// LUT and Grid Padding Helpers
// =============================================================================

/// Pads CLUT grid data for skcms SIMD gather overread (1-2 extra bytes) and aligns to 4-byte boundary.
/// Matches gather safety requirements in `Transform_inl.h`.
fn pad_grid_data(mut data: Vec<u8>, is_16bit: bool) -> Vec<u8> {
    let overread = if is_16bit { 2 } else { 1 };
    let target_len = (data.len() + overread + 3) & !3;
    data.resize(target_len, 0);
    data
}

// =============================================================================
// Multi-Stage Transforms (mft1, mft2, mAB, mBA)
// =============================================================================

/// Direction for LUT-based color transforms.
#[derive(Copy, Clone)]
enum LutDirection {
    A2B,
    B2A,
}

// mft1 and mft2 share a large chunk of data.
// Matches `mft_CommonLayout`, `mft1_Layout`, and `init_tables`.
// MFT matrices are applied before the first set of curves, but must be identity unless the
// input is PCSXYZ. We don't support PCSXYZ profiles, so we ignore this matrix.
// Corresponds to the C++ template `template <typename A2B_or_B2A> static bool read_tag_mft1`.
fn read_tag_mft1(tag: &IccTag, direction: LutDirection) -> Option<ffi::A2BOrB2ATransform> {
    if tag.buf.len() < 48 {
        return None;
    }
    let input_channels = tag.buf[8] as usize;
    let output_channels = tag.buf[9] as usize;
    let grid_point = tag.buf[10];

    match direction {
        LutDirection::A2B => {
            // We require exactly three (ie XYZ/Lab/RGB) output channels
            // We require at least one, and no more than four (ie CMYK) input channels
            // The grid only makes sense with at least two points along each axis
            if output_channels != 3 || input_channels < 1 || input_channels > 4 || grid_point < 2 {
                return None;
            }
        }
        LutDirection::B2A => {
            // For B2A, exactly 3 input channels (XYZ) and 3 (RGB) or 4 (CMYK) output channels.
            if input_channels != 3 || output_channels < 3 || output_channels > 4 || grid_point < 2 {
                return None;
            }
        }
    }

    let mut total_grid_points: u64 = 1;
    let mut grid_points = [0u8; 4];
    for i in 0..input_channels {
        grid_points[i] = grid_point;
        total_grid_points = total_grid_points.checked_mul(grid_point as u64)?;
    }
    if total_grid_points > SKCMS_MAX_GRID_POINTS {
        return None;
    }

    // byte_width is 1, input_table_entries = 256, output_table_entries = 256
    let in_table_bytes = input_channels * 256;
    let grid_bytes = (total_grid_points as usize).checked_mul(output_channels)?;
    let out_table_bytes = output_channels * 256;
    let total_needed = 48 + in_table_bytes + grid_bytes + out_table_bytes;
    if tag.buf.len() < total_needed {
        return None;
    }

    let mut table_offset = 48;
    let mut input_curves = Vec::with_capacity(input_channels);
    for _ in 0..input_channels {
        let slice = &tag.buf[table_offset..table_offset + 256];
        table_offset += 256;
        let mut table_data = Vec::with_capacity(512);
        for &v in slice {
            table_data.push(v);
            table_data.push(v);
        }
        input_curves.push(ffi::Curve {
            table_entries: 256,
            parametric: ffi::TransferFunction::default(),
            table_data,
        });
    }

    let grid_slice = &tag.buf[table_offset..table_offset + grid_bytes];
    table_offset += grid_bytes;
    let grid_data = pad_grid_data(grid_slice.to_vec(), false);

    let mut output_curves = Vec::with_capacity(output_channels);
    for _ in 0..output_channels {
        let slice = &tag.buf[table_offset..table_offset + 256];
        table_offset += 256;
        let mut table_data = Vec::with_capacity(512);
        for &v in slice {
            table_data.push(v);
            table_data.push(v);
        }
        output_curves.push(ffi::Curve {
            table_entries: 256,
            parametric: ffi::TransferFunction::default(),
            table_data,
        });
    }

    Some(ffi::A2BOrB2ATransform {
        input_curves,
        input_channels: input_channels as u32,
        grid_points,
        grid_data,
        is_16bit_grid: false,
        matrix_curves: Vec::new(),
        matrix: ffi::Matrix3x3 {
            vals: [[0.0; 3]; 3],
        },
        matrix_bias: [0.0; 3],
        matrix_channels: 0,
        output_curves,
        output_channels: output_channels as u32,
    })
}

// Matches `mft2_Layout`, `read_tag_mft2`, and `init_tables`.
// Corresponds to the C++ template `template <typename A2B_or_B2A> static bool read_tag_mft2`.
fn read_tag_mft2(tag: &IccTag, direction: LutDirection) -> Option<ffi::A2BOrB2ATransform> {
    if tag.buf.len() < 52 {
        return None;
    }
    let input_channels = tag.buf[8] as usize;
    let output_channels = tag.buf[9] as usize;
    let grid_point = tag.buf[10];

    match direction {
        LutDirection::A2B => {
            if output_channels != 3 || input_channels < 1 || input_channels > 4 || grid_point < 2 {
                return None;
            }
        }
        LutDirection::B2A => {
            if input_channels != 3 || output_channels < 3 || output_channels > 4 || grid_point < 2 {
                return None;
            }
        }
    }

    let in_entries = read_u16(&tag.buf[48..50])? as usize;
    let out_entries = read_u16(&tag.buf[50..52])? as usize;
    // ICC spec mandates that 2 <= table_entries <= 4096
    if in_entries < 2 || in_entries > 4096 || out_entries < 2 || out_entries > 4096 {
        return None;
    }

    let mut total_grid_points: u64 = 1;
    let mut grid_points = [0u8; 4];
    for i in 0..input_channels {
        grid_points[i] = grid_point;
        total_grid_points = total_grid_points.checked_mul(grid_point as u64)?;
    }
    if total_grid_points > SKCMS_MAX_GRID_POINTS {
        return None;
    }

    let in_table_bytes = input_channels * in_entries * 2;
    let grid_bytes = (total_grid_points as usize).checked_mul(output_channels * 2)?;
    let out_table_bytes = output_channels * out_entries * 2;
    let total_needed = 52 + in_table_bytes + grid_bytes + out_table_bytes;
    if tag.buf.len() < total_needed {
        return None;
    }

    let mut table_offset = 52;
    let mut input_curves = Vec::with_capacity(input_channels);
    for _ in 0..input_channels {
        let slice = &tag.buf[table_offset..table_offset + in_entries * 2];
        table_offset += in_entries * 2;
        input_curves.push(ffi::Curve {
            table_entries: in_entries as u32,
            parametric: ffi::TransferFunction::default(),
            table_data: slice.to_vec(),
        });
    }

    let grid_slice = &tag.buf[table_offset..table_offset + grid_bytes];
    table_offset += grid_bytes;
    let grid_data = pad_grid_data(grid_slice.to_vec(), true);

    let mut output_curves = Vec::with_capacity(output_channels);
    for _ in 0..output_channels {
        let slice = &tag.buf[table_offset..table_offset + out_entries * 2];
        table_offset += out_entries * 2;
        output_curves.push(ffi::Curve {
            table_entries: out_entries as u32,
            parametric: ffi::TransferFunction::default(),
            table_data: slice.to_vec(),
        });
    }

    Some(ffi::A2BOrB2ATransform {
        input_curves,
        input_channels: input_channels as u32,
        grid_points,
        grid_data,
        is_16bit_grid: true,
        matrix_curves: Vec::new(),
        matrix: ffi::Matrix3x3 {
            vals: [[0.0; 3]; 3],
        },
        matrix_bias: [0.0; 3],
        matrix_channels: 0,
        output_curves,
        output_channels: output_channels as u32,
    })
}

// mAB and mBA tags use the same encoding, including color lookup tables.
// Matches `mAB_or_mBA_Layout`, `CLUT_Layout`, `read_tag_mab`, and `read_tag_mba`.
fn read_tag_mab_or_mba(
    tag: &IccTag,
    pcs_is_xyz: bool,
    total_profile_len: usize,
    direction: LutDirection,
) -> Option<ffi::A2BOrB2ATransform> {
    if tag.buf.len() < 32 {
        return None;
    }

    let input_channels = tag.buf[8] as usize;
    let output_channels = tag.buf[9] as usize;

    match direction {
        LutDirection::A2B => {
            // We require exactly three (ie XYZ/Lab/RGB) output channels
            // We require at least one, and no more than four (ie CMYK) input channels
            if output_channels != 3 || input_channels < 1 || input_channels > 4 {
                return None;
            }
        }
        LutDirection::B2A => {
            // Require exactly 3 inputs (XYZ) and 3 (RGB) or 4 (CMYK) outputs.
            if input_channels != 3 || output_channels < 3 || output_channels > 4 {
                return None;
            }
        }
    }

    let b_curve_offset = read_u32(&tag.buf[12..16])? as usize;
    let matrix_offset = read_u32(&tag.buf[16..20])? as usize;
    let m_curve_offset = read_u32(&tag.buf[20..24])? as usize;
    let clut_offset = read_u32(&tag.buf[24..28])? as usize;
    let a_curve_offset = read_u32(&tag.buf[28..32])? as usize;

    // "B" curves must be present
    if b_curve_offset == 0 {
        return None;
    }

    // For A2B, "B" curves are output (PCS); for B2A, "B" curves are input (PCS).
    let b_curves_count = match direction {
        LutDirection::A2B => output_channels,
        LutDirection::B2A => input_channels,
    };
    let b_curves = read_curves(tag.buf, b_curve_offset, b_curves_count)?;

    let (matrix_channels, matrix_curves, matrix, matrix_bias) = if m_curve_offset != 0 {
        if matrix_offset == 0 || tag.buf.len() < matrix_offset + 48 {
            return None;
        }
        // For A2B, matrix channels is tied to output_channels (3);
        // for B2A, matrix channels is tied to input_channels (3).
        let matrix_channels = match direction {
            LutDirection::A2B => output_channels,
            LutDirection::B2A => input_channels,
        };
        let matrix_curves = read_curves(tag.buf, m_curve_offset, matrix_channels)?;

        let encoding_factor = if pcs_is_xyz {
            match direction {
                LutDirection::A2B => 65535.0 / 32768.0,
                LutDirection::B2A => 32768.0 / 65535.0,
            }
        } else {
            1.0
        };
        let mtx_buf = &tag.buf[matrix_offset..];
        let matrix = ffi::Matrix3x3 {
            vals: [
                [
                    encoding_factor * read_fixed(&mtx_buf[0..4])?,
                    encoding_factor * read_fixed(&mtx_buf[4..8])?,
                    encoding_factor * read_fixed(&mtx_buf[8..12])?,
                ],
                [
                    encoding_factor * read_fixed(&mtx_buf[12..16])?,
                    encoding_factor * read_fixed(&mtx_buf[16..20])?,
                    encoding_factor * read_fixed(&mtx_buf[20..24])?,
                ],
                [
                    encoding_factor * read_fixed(&mtx_buf[24..28])?,
                    encoding_factor * read_fixed(&mtx_buf[28..32])?,
                    encoding_factor * read_fixed(&mtx_buf[32..36])?,
                ],
            ],
        };
        let matrix_bias = [
            encoding_factor * read_fixed(&mtx_buf[36..40])?,
            encoding_factor * read_fixed(&mtx_buf[40..44])?,
            encoding_factor * read_fixed(&mtx_buf[44..48])?,
        ];
        (matrix_channels, matrix_curves, matrix, matrix_bias)
    } else {
        if matrix_offset != 0 {
            return None;
        }
        (
            0,
            Vec::new(),
            ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            [0.0; 3],
        )
    };

    // "A" curves and CLUT must be used together
    let (final_channels, a_curves, grid_points, grid_data, is_16bit_grid) =
        if a_curve_offset != 0 {
            if clut_offset == 0 || tag.buf.len() < clut_offset + 20 {
                return None;
            }
            // For A2B, "A" curves are input (Device); for B2A, "A" curves are output (Device).
            let a_curves_count = match direction {
                LutDirection::A2B => input_channels,
                LutDirection::B2A => output_channels,
            };
            let a_curves = read_curves(tag.buf, a_curve_offset, a_curves_count)?;
            let clut_buf = &tag.buf[clut_offset..];
            let grid_byte_width = clut_buf[16] as usize;
            if grid_byte_width != 1 && grid_byte_width != 2 {
                return None;
            }

            let mut grid_points = [0u8; 4];
            let mut total_grid_points: u64 = 1;
            for i in 0..input_channels {
                grid_points[i] = clut_buf[i];
                // The grid only makes sense with at least two points along each axis
                if grid_points[i] < 2 {
                    return None;
                }
                total_grid_points = total_grid_points.checked_mul(grid_points[i] as u64)?;
            }
            if total_grid_points > SKCMS_MAX_GRID_POINTS {
                return None;
            }

            let grid_size = (output_channels * grid_byte_width).checked_mul(total_grid_points as usize)?;
            if tag.buf.len() < clut_offset + 20 + grid_size {
                return None;
            }

            // gather_24 and gather_48 read 1 or 2 extra bytes.
            // We must ensure that those extra bytes are within the provided buffer limit.
            let slack = if output_channels == 3 {
                if grid_byte_width == 1 { 1 } else { 2 }
            } else {
                0
            };
            if tag.offset + clut_offset + 20 + grid_size + slack > total_profile_len {
                return None;
            }

            let is_16bit_grid = grid_byte_width == 2;
            let raw_grid = &tag.buf[clut_offset + 20..clut_offset + 20 + grid_size];
            let grid_data = pad_grid_data(raw_grid.to_vec(), is_16bit_grid);

            (
                match direction {
                    LutDirection::A2B => input_channels,
                    LutDirection::B2A => output_channels,
                },
                a_curves,
                grid_points,
                grid_data,
                is_16bit_grid,
            )
        } else {
            if clut_offset != 0 {
                return None;
            }

            // If there is no CLUT, the number of input and output channels must match
            if input_channels != output_channels {
                return None;
            }

            // Zero out channels to signal that we're skipping this stage
            // (input_channels for A2B, output_channels for B2A)
            (0, Vec::new(), [0; 4], Vec::new(), false)
        };

    let (input_curves, final_input_channels, output_curves, final_output_channels) =
        match direction {
            LutDirection::A2B => (a_curves, final_channels, b_curves, output_channels),
            LutDirection::B2A => (b_curves, input_channels, a_curves, final_channels),
        };

    Some(ffi::A2BOrB2ATransform {
        input_curves,
        input_channels: final_input_channels as u32,
        grid_points,
        grid_data,
        is_16bit_grid,
        matrix_curves,
        matrix,
        matrix_bias,
        matrix_channels: matrix_channels as u32,
        output_curves,
        output_channels: final_output_channels as u32,
    })
}

/// Dispatches LUT tag reading based on tag type (matches `read_a2b` and `read_b2a`).
fn read_lut(
    tag: &IccTag,
    pcs_is_xyz: bool,
    total_profile_len: usize,
    direction: LutDirection,
) -> Option<ffi::A2BOrB2ATransform> {
    match (tag.tag_type, direction) {
        (SKCMS_SIGNATURE_MFT1, _) => read_tag_mft1(tag, direction),
        (SKCMS_SIGNATURE_MFT2, _) => read_tag_mft2(tag, direction),
        (SKCMS_SIGNATURE_M_AB, LutDirection::A2B) | (SKCMS_SIGNATURE_M_BA, LutDirection::B2A) => {
            read_tag_mab_or_mba(tag, pcs_is_xyz, total_profile_len, direction)
        }
        _ => None,
    }
}

/// Parses CICP metadata (matches `read_cicp` and `CICP_Layout`).
fn read_cicp(tag: &IccTag) -> Option<ffi::Cicp> {
    if tag.tag_type != SKCMS_SIGNATURE_CICP || tag.buf.len() < 12 {
        return None;
    }
    Some(ffi::Cicp {
        color_primaries: tag.buf[8],
        transfer_characteristics: tag.buf[9],
        matrix_coefficients: tag.buf[10],
        video_full_range_flag: tag.buf[11],
    })
}

/// Validates whether a profile is usable as a source profile (matches `usable_as_src`).
fn usable_as_src(profile: &ffi::IccProfile) -> bool {
    profile.has_a2b || (profile.has_trc && profile.has_to_xyzd50)
}

// =============================================================================
// Top-Level Profile Parser
// =============================================================================

/// Parses ICC profile from `data`. If successful, returns `true`
/// and writes result to `out`. If failure, returns `false`.
/// (Matches `skcms_ParseWithA2BPriority`).
pub fn parse_icc_profile(data: &[u8], out: &mut ffi::IccProfile) -> bool {
    if data.len() < SAFE_HEADER_SIZE {
        return false;
    }

    let header = &data[..SAFE_HEADER_SIZE];
    let size = match read_u32(&header[0..4]) {
        Some(s) => s as usize,
        None => return false,
    };
    let version = match read_u32(&header[8..12]) {
        Some(v) => v,
        None => return false,
    };
    let data_color_space = match read_u32(&header[16..20]) {
        Some(cs) => cs,
        None => return false,
    };
    let pcs = match read_u32(&header[20..24]) {
        Some(p) => p,
        None => return false,
    };
    let signature = match read_u32(&data[36..40]) {
        Some(s) => s,
        None => return false,
    };
    let illuminant_x = match read_fixed(&data[68..72]) {
        Some(v) => v,
        None => return false,
    };
    let illuminant_y = match read_fixed(&data[72..76]) {
        Some(v) => v,
        None => return false,
    };
    let illuminant_z = match read_fixed(&data[76..80]) {
        Some(v) => v,
        None => return false,
    };
    let tag_count = match read_u32(&data[128..132]) {
        Some(c) => c as usize,
        None => return false,
    };

    let tag_table_size = match tag_count.checked_mul(SAFE_TAG_ENTRY_SIZE) {
        Some(s) => s,
        None => return false,
    };

    // Validate signature, size (smaller than buffer, large enough to hold tag table),
    // and major version
    if signature != SKCMS_SIGNATURE_ACSP
        || size > data.len()
        || size < SAFE_HEADER_SIZE + tag_table_size
        || (version >> 24) > 4
    {
        return false;
    }

    // Validate that illuminant is D50 white
    if (illuminant_x - 0.9642).abs() > 0.0100
        || (illuminant_y - 1.0000).abs() > 0.0100
        || (illuminant_z - 0.8249).abs() > 0.0100
    {
        return false;
    }

    // Validate that all tag entries have sane offset + size
    let tag_table = match data.get(SAFE_HEADER_SIZE..) {
        Some(t) => t,
        None => return false,
    };
    for i in 0..tag_count {
        let entry = match tag_table.get(i * SAFE_TAG_ENTRY_SIZE..(i + 1) * SAFE_TAG_ENTRY_SIZE) {
            Some(e) => e,
            None => return false,
        };
        let tag_offset = match read_u32(&entry[4..8]) {
            Some(o) => o as usize,
            None => return false,
        };
        let tag_size = match read_u32(&entry[8..12]) {
            Some(s) => s as usize,
            None => return false,
        };
        let tag_end = match tag_offset.checked_add(tag_size) {
            Some(e) => e,
            None => return false,
        };
        if tag_size < 4 || tag_end > size {
            return false;
        }
    }

    if pcs != SKCMS_SIGNATURE_XYZ && pcs != SKCMS_SIGNATURE_LAB {
        return false;
    }

    let pcs_is_xyz = match pcs {
        SKCMS_SIGNATURE_XYZ => true,
        SKCMS_SIGNATURE_LAB => false,
        _ => return false,
    };

    out.data_color_space = data_color_space;
    out.connection_space = pcs;

    out.has_to_xyzd50 = false;
    out.has_trc = false;

    // Pre-parse commonly used tags.
    if data_color_space == SKCMS_SIGNATURE_GRAY {
        if let Some(ktrc) = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_K_TRC) {
            let (curve, _) = match read_curve(ktrc.buf) {
                Some(c) => c,
                None => return false, // Malformed tag
            };
            out.trc_r = curve.clone();
            out.trc_g = curve.clone();
            out.trc_b = curve;
            out.has_trc = true;

            if pcs_is_xyz {
                out.to_xyzd50 = ffi::Matrix3x3 {
                    vals: [
                        [illuminant_x, 0.0, 0.0],
                        [0.0, illuminant_y, 0.0],
                        [0.0, 0.0, illuminant_z],
                    ],
                };
                out.has_to_xyzd50 = true;
            }
        }
    } else {
        let r_trc = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_R_TRC);
        let g_trc = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_G_TRC);
        let b_trc = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_B_TRC);
        if let (Some(r), Some(g), Some(b)) = (r_trc, g_trc, b_trc) {
            let r_curve = match read_curve(r.buf) {
                Some((c, _)) => c,
                None => return false, // Malformed TRC tags
            };
            let g_curve = match read_curve(g.buf) {
                Some((c, _)) => c,
                None => return false, // Malformed TRC tags
            };
            let b_curve = match read_curve(b.buf) {
                Some((c, _)) => c,
                None => return false, // Malformed TRC tags
            };
            out.trc_r = r_curve;
            out.trc_g = g_curve;
            out.trc_b = b_curve;
            out.has_trc = true;
        }

        let r_xyz = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_R_XYZ);
        let g_xyz = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_G_XYZ);
        let b_xyz = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_B_XYZ);
        if let (Some(r), Some(g), Some(b)) = (r_xyz, g_xyz, b_xyz) {
            let matrix = match read_to_xyzd50(&r, &g, &b) {
                Some(m) => m,
                None => return false, // Malformed XYZ tags
            };
            out.to_xyzd50 = matrix;
            out.has_to_xyzd50 = true;
        }
    }

    // enum { perceptual, relative_colormetric, saturation }
    out.has_a2b = false;
    for &sig in &[SKCMS_SIGNATURE_A2B0, SKCMS_SIGNATURE_A2B1] {
        if let Some(tag) = get_tag_by_signature(data, tag_count, sig) {
            let a2b = match read_lut(&tag, pcs_is_xyz, data.len(), LutDirection::A2B) {
                Some(a) => a,
                None => return false, // Malformed A2B tag
            };
            out.a2b = a2b;
            out.has_a2b = true;
            break;
        }
    }

    out.has_b2a = false;
    for &sig in &[SKCMS_SIGNATURE_B2A0, SKCMS_SIGNATURE_B2A1] {
        if let Some(tag) = get_tag_by_signature(data, tag_count, sig) {
            let b2a = match read_lut(&tag, pcs_is_xyz, data.len(), LutDirection::B2A) {
                Some(b) => b,
                None => return false, // Malformed B2A tag
            };
            out.b2a = b2a;
            out.has_b2a = true;
            break;
        }
    }

    out.has_cicp = false;
    if let Some(tag) = get_tag_by_signature(data, tag_count, SKCMS_SIGNATURE_CICP) {
        let cicp = match read_cicp(&tag) {
            Some(c) => c,
            None => return false, // Malformed CICP tag
        };
        out.cicp = cicp;
        out.has_cicp = true;
    }

    usable_as_src(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn empty_curve() -> ffi::Curve {
        ffi::Curve {
            table_entries: 0,
            parametric: ffi::TransferFunction::default(),
            table_data: Vec::new(),
        }
    }

    fn empty_lut() -> ffi::A2BOrB2ATransform {
        ffi::A2BOrB2ATransform {
            input_curves: Vec::new(),
            input_channels: 0,
            grid_points: [0; 4],
            grid_data: Vec::new(),
            is_16bit_grid: false,
            matrix_curves: Vec::new(),
            matrix: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            matrix_bias: [0.0; 3],
            matrix_channels: 0,
            output_curves: Vec::new(),
            output_channels: 0,
        }
    }

    fn empty_icc_profile() -> ffi::IccProfile {
        ffi::IccProfile {
            data_color_space: u32::from_be_bytes(*b"RGB "),
            connection_space: SKCMS_SIGNATURE_XYZ,
            to_xyzd50: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            has_to_xyzd50: false,
            trc_r: empty_curve(),
            trc_g: empty_curve(),
            trc_b: empty_curve(),
            has_trc: false,
            cicp: ffi::Cicp {
                color_primaries: 0,
                transfer_characteristics: 0,
                matrix_coefficients: 0,
                video_full_range_flag: 0,
            },
            has_cicp: false,
            a2b: empty_lut(),
            has_a2b: false,
            b2a: empty_lut(),
            has_b2a: false,
        }
    }

    #[test]
    fn test_empty_profile() {
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&[], &mut out);
        assert!(!result);
    }

    #[test]
    fn test_too_short() {
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&[0; 39], &mut out);
        assert!(!result);
    }

    #[test]
    fn test_invalid_signature() {
        let mut data = vec![0u8; 132];
        data[36..40].copy_from_slice(b"badd");
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }

    #[test]
    fn test_valid_signature_invalid_profile() {
        let mut data = vec![0u8; 132];
        data[36..40].copy_from_slice(b"acsp");
        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }
}
