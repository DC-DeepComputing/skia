// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#![cfg_attr(not(test), allow(dead_code))]

use crate::ffi;
use crate::icc_reader::Reader;

const XYZ_SIGNATURE: u32 = u32::from_be_bytes(*b"XYZ ");
const CURVE_SIGNATURE: u32 = u32::from_be_bytes(*b"curv");
const PARAMETRIC_CURVE_SIGNATURE: u32 = u32::from_be_bytes(*b"para");
const MAX_TRC_ENTRIES: usize = 40_000;

#[derive(Clone, Copy)]
pub(crate) enum EmptyCurve {
    Reject,
    Identity,
}

pub(crate) struct ParsedCurve {
    pub(crate) curve: ffi::Curve,
    pub(crate) bytes_used: usize,
}

fn transfer_function(
    g: f32,
    a: f32,
    b: f32,
    c: f32,
    d: f32,
    e: f32,
    f: f32,
) -> ffi::TransferFunction {
    ffi::TransferFunction { g, a, b, c, d, e, f }
}

fn identity_curve() -> ffi::Curve {
    ffi::Curve {
        table_entries: 0,
        parametric: transfer_function(1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0),
        table_data: Vec::new(),
    }
}

pub(crate) fn parse_xyz(data: &[u8]) -> Option<[f32; 3]> {
    let reader = Reader::new(data);
    if reader.be_u32(0)? != XYZ_SIGNATURE {
        return None;
    }

    Some([
        reader.s15_fixed16(8)?,
        reader.s15_fixed16(12)?,
        reader.s15_fixed16(16)?,
    ])
}

pub(crate) fn parse_curve(data: &[u8], empty: EmptyCurve) -> Option<ParsedCurve> {
    let reader = Reader::new(data);
    match reader.be_u32(0)? {
        CURVE_SIGNATURE => parse_table_curve(reader, empty),
        PARAMETRIC_CURVE_SIGNATURE => parse_parametric_curve(reader),
        _ => None,
    }
}

fn parse_table_curve(reader: Reader<'_>, empty: EmptyCurve) -> Option<ParsedCurve> {
    let entries = reader.be_u32(8)? as usize;
    if entries > MAX_TRC_ENTRIES {
        return None;
    }

    let table_len = entries.checked_mul(2)?;
    let bytes_used = 12usize.checked_add(table_len)?;
    let table = reader.bytes(12, table_len)?;

    let curve = match entries {
        0 => match empty {
            EmptyCurve::Reject => return None,
            EmptyCurve::Identity => identity_curve(),
        },
        1 => {
            let gamma = u16::from_be_bytes(table.try_into().ok()?) as f32 / 256.0;
            ffi::Curve {
                table_entries: 0,
                parametric: transfer_function(gamma, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0),
                table_data: Vec::new(),
            }
        }
        _ => ffi::Curve {
            table_entries: entries as u32,
            parametric: ffi::TransferFunction::default(),
            table_data: table.to_vec(),
        },
    };

    Some(ParsedCurve { curve, bytes_used })
}

fn parse_parametric_curve(reader: Reader<'_>) -> Option<ParsedCurve> {
    let function_type = reader.be_u16(8)? as usize;
    const PARAMETER_COUNTS: [usize; 5] = [1, 3, 4, 5, 7];
    let parameter_count = *PARAMETER_COUNTS.get(function_type)?;
    let bytes_used = 12usize.checked_add(parameter_count.checked_mul(4)?)?;
    reader.bytes(0, bytes_used)?;

    let mut parameters = [0.0f32; 7];
    for (index, parameter) in parameters[..parameter_count].iter_mut().enumerate() {
        *parameter = reader.s15_fixed16(12 + index * 4)?;
    }

    let [g, a, b, c, d, e, f] = parameters;
    let parametric = match function_type {
        0 => transfer_function(g, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0),
        1 => {
            if a == 0.0 {
                return None;
            }
            transfer_function(g, a, b, 0.0, -b / a, 0.0, 0.0)
        }
        2 => {
            if a == 0.0 {
                return None;
            }
            transfer_function(g, a, b, 0.0, -b / a, c, c)
        }
        3 => transfer_function(g, a, b, c, d, 0.0, 0.0),
        4 => transfer_function(g, a, b, c, d, e, f),
        _ => return None,
    };

    Some(ParsedCurve {
        curve: ffi::Curve {
            table_entries: 0,
            parametric,
            table_data: Vec::new(),
        },
        bytes_used,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    fn fixed(value: f32) -> [u8; 4] {
        ((value * 65536.0) as i32).to_be_bytes()
    }

    fn parametric(function_type: u16, values: &[f32]) -> Vec<u8> {
        let mut data = Vec::from(*b"para\0\0\0\0");
        data.extend(function_type.to_be_bytes());
        data.extend([0, 0]);
        for value in values {
            data.extend(fixed(*value));
        }
        data
    }

    #[test]
    fn xyz_uses_signed_fixed_point() {
        let mut data = Vec::from(*b"XYZ \0\0\0\0");
        data.extend(fixed(0.5));
        data.extend(fixed(1.0));
        data.extend(fixed(-0.25));
        assert_eq!(parse_xyz(&data), Some([0.5, 1.0, -0.25]));
        assert_eq!(parse_xyz(&data[..19]), None);
    }

    #[test]
    fn table_curve_preserves_big_endian_bytes() {
        let data = [
            b'c', b'u', b'r', b'v', 0, 0, 0, 0, 0, 0, 0, 3, 0x00, 0x01, 0x12, 0x34, 0xfe,
            0xdc,
        ];
        let parsed = parse_curve(&data, EmptyCurve::Reject).unwrap();
        assert_eq!(parsed.bytes_used, data.len());
        assert_eq!(parsed.curve.table_entries, 3);
        assert_eq!(parsed.curve.table_data, data[12..]);
    }

    #[test]
    fn empty_curve_is_context_sensitive() {
        let data = [b'c', b'u', b'r', b'v', 0, 0, 0, 0, 0, 0, 0, 0];
        assert!(parse_curve(&data, EmptyCurve::Reject).is_none());

        let parsed = parse_curve(&data, EmptyCurve::Identity).unwrap();
        assert_eq!(parsed.curve.table_entries, 0);
        assert_eq!(parsed.curve.parametric.g, 1.0);
        assert_eq!(parsed.curve.parametric.a, 1.0);
    }

    #[test]
    fn single_table_entry_is_gamma() {
        let data = [b'c', b'u', b'r', b'v', 0, 0, 0, 0, 0, 0, 0, 1, 0x02, 0x33];
        let parsed = parse_curve(&data, EmptyCurve::Reject).unwrap();
        assert_eq!(parsed.curve.table_entries, 0);
        assert!((parsed.curve.parametric.g - 2.19921875).abs() < f32::EPSILON);
    }

    #[test]
    fn parametric_types_are_normalized() {
        let type_0 = parse_curve(&parametric(0, &[2.0]), EmptyCurve::Reject).unwrap();
        assert_eq!(type_0.curve.parametric.g, 2.0);
        assert_eq!(type_0.curve.parametric.a, 1.0);
        assert_eq!(type_0.curve.parametric.c, 0.0);

        let type_1 = parse_curve(&parametric(1, &[2.0, 4.0, 2.0]), EmptyCurve::Reject).unwrap();
        assert_eq!(type_1.curve.parametric.d, -0.5);

        let type_2 =
            parse_curve(&parametric(2, &[2.0, 4.0, 2.0, 0.25]), EmptyCurve::Reject).unwrap();
        assert_eq!(type_2.curve.parametric.d, -0.5);
        assert_eq!(type_2.curve.parametric.e, 0.25);
        assert_eq!(type_2.curve.parametric.f, 0.25);

        let type_3 = parse_curve(
            &parametric(3, &[2.0, 1.0, 2.0, 3.0, 4.0]),
            EmptyCurve::Reject,
        )
        .unwrap();
        assert_eq!(type_3.curve.parametric.c, 3.0);
        assert_eq!(type_3.curve.parametric.d, 4.0);

        let type_4 = parse_curve(
            &parametric(4, &[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]),
            EmptyCurve::Reject,
        )
        .unwrap();
        assert_eq!(type_4.curve.parametric.f, 7.0);
    }

    #[test]
    fn malformed_curves_are_rejected() {
        assert!(parse_curve(b"nope", EmptyCurve::Reject).is_none());
        assert!(parse_curve(&parametric(5, &[1.0]), EmptyCurve::Reject).is_none());
        assert!(parse_curve(&parametric(1, &[2.0, 0.0, 1.0]), EmptyCurve::Reject).is_none());

        let mut truncated = parametric(4, &[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]);
        truncated.pop();
        assert!(parse_curve(&truncated, EmptyCurve::Reject).is_none());

        let mut oversized = Vec::from(*b"curv\0\0\0\0");
        oversized.extend(((MAX_TRC_ENTRIES + 1) as u32).to_be_bytes());
        assert!(parse_curve(&oversized, EmptyCurve::Reject).is_none());
    }
}
