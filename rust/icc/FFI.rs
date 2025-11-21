// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser FFI bindings.
//!
//! Provides C++ bindings for parsing ICC profiles using moxcms. All parsing
//! happens in Rust for memory safety, then validated data is converted to
//! skcms-compatible structures for color transformations.

use moxcms::{ColorProfile, DataColorSpace};

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    /// 3x3 matrix for color space transforms (matches skcms_Matrix3x3).
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }

    /// Transfer function parameters (matches skcms_TransferFunction).
    #[derive(Clone, Copy)]
    struct TransferFunction {
        g: f32, 
        a: f32, 
        b: f32, 
        c: f32, 
        d: f32, 
        e: f32, 
        f: f32,
    }

    /// CICP color metadata (matches skcms_CICP).
    #[derive(Clone, Copy)]
    struct Cicp {
        color_primaries: u8,
        transfer_characteristics: u8,
        matrix_coefficients: u8,
        video_full_range_flag: u8,
    }

    /// Curve for LUT transforms - parametric or table-based (matches skcms_Curve).
    struct Curve {
        table_entries: u32,  // 0 = parametric, >0 = table
        parametric: TransferFunction,
        table_data: Vec<u8>,  // u16 values as bytes (little-endian) for tables
    }

    /// Device-to-PCS transform (matches skcms_A2B).
    struct A2B {
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

    /// PCS-to-device transform (matches skcms_B2A).
    struct B2A {
        /// Required: 3 1D "B" curves. Always present.
        input_curves: Vec<Curve>,
        input_channels: u32,

        /// Optional: 3x4 matrix followed by 3 1D "M" curves.
        /// If matrix_channels == 0, matrix and curves are skipped.
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_curves: Vec<Curve>,
        matrix_channels: u32,

        /// Optional: N-D CLUT followed by N 1D "A" curves.
        /// If output_channels == 0, CLUT and curves are skipped.
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        is_16bit_grid: bool,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    /// Parsed ICC profile data.
    struct IccProfile {
        data_color_space: ColorSpace,
        connection_space: ColorSpace,
        rendering_intent: RenderingIntent,
        to_xyzd50: Matrix3x3,
        has_to_xyzd50: bool,
        trc: [TransferFunction; 3],
        has_trc: bool,
        cicp: Cicp,
        has_cicp: bool,
        a2b: A2B,
        has_a2b: bool,
        b2a: B2A,
        has_b2a: bool,
    }

    /// Color space types.
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum ColorSpace {
        Unknown = 0,
        XYZ = 1,
        Lab = 2,
        Luv = 3,
        YCbCr = 4,
        Yxy = 5,
        RGB = 6,
        Gray = 7,
        HSV = 8,
        HLS = 9,
        CMYK = 10,
        CMY = 11,
        Color2 = 12,
        Color3 = 13,
        Color4 = 14,
        Color5 = 15,
        Color6 = 16,
        Color7 = 17,
        Color8 = 18,
        Color9 = 19,
        Color10 = 20,
        Color11 = 21,
        Color12 = 22,
        Color13 = 23,
        Color14 = 24,
        Color15 = 25,
    }

    /// ICC rendering intent.
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum RenderingIntent {
        Perceptual = 0,
        RelativeColorimetric = 1,
        Saturation = 2,
        AbsoluteColorimetric = 3,
    }

    extern "Rust" {
        /// Parses ICC profile from `data`. If successful, returns `true`
        /// and writes result to `out`. If failure, returns `false`.
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}

/// Parses ICC profile from `data`. If successful, returns `true`
/// and writes result to `out`. If failure, returns `false`.
pub fn parse_icc_profile(data: &[u8], out: &mut ffi::IccProfile) -> bool {
    use ffi::{ColorSpace, RenderingIntent};

    // Parse with moxcms (it validates size, signature, etc.)
    let profile = match ColorProfile::new_from_slice(data) {
        Ok(profile) => profile,
        Err(_) => return false,
    };
    // Convert moxcms DataColorSpace to our ColorSpace enum
    // NOTE: This match is intentionally exhaustive (no catch-all pattern).
    // If moxcms adds new DataColorSpace variants, this will fail to compile,
    // forcing us to consciously decide how to map the new variant.
    let data_color_space = match profile.color_space {
        DataColorSpace::Xyz => ColorSpace::XYZ,
        DataColorSpace::Lab => ColorSpace::Lab,
        DataColorSpace::Luv => ColorSpace::Luv,
        DataColorSpace::YCbr => ColorSpace::YCbCr,
        DataColorSpace::Yxy => ColorSpace::Yxy,
        DataColorSpace::Rgb => ColorSpace::RGB,
        DataColorSpace::Gray => ColorSpace::Gray,
        DataColorSpace::Hsv => ColorSpace::HSV,
        DataColorSpace::Hls => ColorSpace::HLS,
        DataColorSpace::Cmyk => ColorSpace::CMYK,
        DataColorSpace::Cmy => ColorSpace::CMY,
        // Multi-channel color spaces recognized by skcms
        DataColorSpace::Color2 => ColorSpace::Color2,
        DataColorSpace::Color3 => ColorSpace::Color3,
        DataColorSpace::Color4 => ColorSpace::Color4,
        DataColorSpace::Color5 => ColorSpace::Color5,
        DataColorSpace::Color6 => ColorSpace::Color6,
        DataColorSpace::Color7 => ColorSpace::Color7,
        DataColorSpace::Color8 => ColorSpace::Color8,
        DataColorSpace::Color9 => ColorSpace::Color9,
        DataColorSpace::Color10 => ColorSpace::Color10,
        DataColorSpace::Color11 => ColorSpace::Color11,
        DataColorSpace::Color12 => ColorSpace::Color12,
        DataColorSpace::Color13 => ColorSpace::Color13,
        DataColorSpace::Color14 => ColorSpace::Color14,
        DataColorSpace::Color15 => ColorSpace::Color15,
    };

    let connection_space = match profile.pcs {
        DataColorSpace::Xyz => ColorSpace::XYZ,
        DataColorSpace::Lab => ColorSpace::Lab,
        _ => ColorSpace::Unknown,
    };

    let rendering_intent = match profile.rendering_intent as u32 {
        0 => RenderingIntent::Perceptual,
        1 => RenderingIntent::RelativeColorimetric,
        2 => RenderingIntent::Saturation,
        3 => RenderingIntent::AbsoluteColorimetric,
        _ => RenderingIntent::Perceptual,
    };

    out.data_color_space = data_color_space;
    out.connection_space = connection_space;
    out.rendering_intent = rendering_intent;

    let matrix = profile.colorant_matrix();
    out.to_xyzd50 = ffi::Matrix3x3 {
        vals: [
            [matrix.v[0][0] as f32, matrix.v[0][1] as f32, matrix.v[0][2] as f32],
            [matrix.v[1][0] as f32, matrix.v[1][1] as f32, matrix.v[1][2] as f32],
            [matrix.v[2][0] as f32, matrix.v[2][1] as f32, matrix.v[2][2] as f32],
        ],
    };
    out.has_to_xyzd50 = matrix.v[0][0] != 0.0
        || matrix.v[0][1] != 0.0
        || matrix.v[0][2] != 0.0
        || matrix.v[1][0] != 0.0;

    out.has_trc = false;
    if let Some(gray_trc) = &profile.gray_trc {
        if let Some(tf) = convert_trc_to_transfer_function(gray_trc) {
            out.trc = [tf, tf, tf];
            out.has_trc = true;
        }
    } else if let (Some(r_trc), Some(g_trc), Some(b_trc)) = (
        &profile.red_trc,
        &profile.green_trc,
        &profile.blue_trc,
    ) {
        if let (Some(r_tf), Some(g_tf), Some(b_tf)) = (
            convert_trc_to_transfer_function(r_trc),
            convert_trc_to_transfer_function(g_trc),
            convert_trc_to_transfer_function(b_trc),
        ) {
            out.trc = [r_tf, g_tf, b_tf];
            out.has_trc = true;
        }
    }

    out.has_cicp = false;
    if let Some(cicp) = &profile.cicp {
        out.cicp = ffi::Cicp {
            color_primaries: cicp.color_primaries as u8,
            transfer_characteristics: cicp.transfer_characteristics as u8,
            matrix_coefficients: cicp.matrix_coefficients as u8,
            video_full_range_flag: if cicp.full_range { 1 } else { 0 },
        };
        out.has_cicp = true;
    }

    // Extract A2B transform (device-to-PCS with LUTs)
    // Prefer perceptual, then colorimetric, then saturation
    out.has_a2b = false;
    if let Some(ref lut) = profile.lut_a_to_b_perceptual
        .as_ref()
        .or(profile.lut_a_to_b_colorimetric.as_ref())
        .or(profile.lut_a_to_b_saturation.as_ref())
    {
        if let Some(a2b) = convert_to_a2b(lut) {
            out.a2b = a2b;
            out.has_a2b = true;
        }
    }

    out.has_b2a = false;
    if let Some(ref lut) = profile.lut_b_to_a_perceptual
        .as_ref()
        .or(profile.lut_b_to_a_colorimetric.as_ref())
        .or(profile.lut_b_to_a_saturation.as_ref())
    {
        if let Some(b2a_data) = convert_to_a2b(lut) {
            out.b2a = ffi::B2A {
                input_curves: b2a_data.output_curves,
                input_channels: b2a_data.output_channels,
                matrix: b2a_data.matrix,
                matrix_bias: b2a_data.matrix_bias,
                matrix_curves: b2a_data.matrix_curves,
                matrix_channels: b2a_data.matrix_channels,
                grid_points: b2a_data.grid_points,
                grid_data: b2a_data.grid_data,
                is_16bit_grid: b2a_data.is_16bit_grid,
                output_curves: b2a_data.input_curves,
                output_channels: b2a_data.input_channels,
            };
            out.has_b2a = true;
        }
    }

    true
}

/// Convert moxcms ToneReprCurve to FFI Curve structure.
/// Supports both parametric and table-based curves.
/// Returns None if the curve is empty or invalid.
fn convert_to_curve(trc: &moxcms::ToneReprCurve) -> Option<ffi::Curve> {
    use moxcms::ToneReprCurve;
    
    match trc {
        ToneReprCurve::Parametric(params) => {
            if params.len() >= 7 {
                Some(ffi::Curve {
                    table_entries: 0,  // 0 indicates parametric
                    parametric: ffi::TransferFunction {
                        g: params[0],
                        a: params[1],
                        b: params[2],
                        c: params[3],
                        d: params[4],
                        e: params[5],
                        f: params[6],
                    },
                    table_data: Vec::new(),
                })
            } else {
                None
            }
        }
        ToneReprCurve::Lut(table) => {
            if table.is_empty() {
                return None;
            }
            
            // Convert Vec<u16> to Vec<u8> (little-endian)
            let mut data = Vec::with_capacity(table.len() * 2);
            for &value in table {
                data.push(value as u8);           // low byte
                data.push((value >> 8) as u8);    // high byte
            }
            
            Some(ffi::Curve {
                table_entries: table.len() as u32,
                parametric: ffi::TransferFunction {
                    g: 0.0,
                    a: 0.0,
                    b: 0.0,
                    c: 0.0,
                    d: 0.0,
                    e: 0.0,
                    f: 0.0,
                },
                table_data: data,
            })
        }
    }
}

/// Convert moxcms LutWarehouse to A2B structure.
/// Returns None if the LUT cannot be converted.
fn convert_to_a2b(lut: &moxcms::LutWarehouse) -> Option<ffi::A2B> {
    use moxcms::{LutStore, LutWarehouse};
    
    match lut {
        LutWarehouse::Multidimensional(mdt) => {
            // Convert input curves (A curves)
            let input_curves: Vec<ffi::Curve> = mdt.a_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();
            
            // Grid data
            let (grid_data, is_16bit_grid) = if let Some(ref clut) = mdt.clut {
                match clut {
                    LutStore::Store8(data) => (data.clone(), false),
                    LutStore::Store16(data) => {
                        // Convert Vec<u16> to Vec<u8> (little-endian)
                        let mut bytes = Vec::with_capacity(data.len() * 2);
                        for &value in data {
                            bytes.push(value as u8);
                            bytes.push((value >> 8) as u8);
                        }
                        (bytes, true)
                    }
                }
            } else {
                (Vec::new(), false)
            };
            
            // Matrix curves (M curves)
            let matrix_curves: Vec<ffi::Curve> = mdt.m_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();
            
            // Matrix (3x3 part) and bias
            let matrix = ffi::Matrix3x3 {
                vals: [
                    [mdt.matrix.v[0][0] as f32, mdt.matrix.v[0][1] as f32, mdt.matrix.v[0][2] as f32],
                    [mdt.matrix.v[1][0] as f32, mdt.matrix.v[1][1] as f32, mdt.matrix.v[1][2] as f32],
                    [mdt.matrix.v[2][0] as f32, mdt.matrix.v[2][1] as f32, mdt.matrix.v[2][2] as f32],
                ],
            };
            let matrix_bias = [
                mdt.bias.v[0] as f32,
                mdt.bias.v[1] as f32,
                mdt.bias.v[2] as f32,
            ];
            
            // Output curves (B curves)
            let output_curves: Vec<ffi::Curve> = mdt.b_curves
                .iter()
                .filter_map(|c| convert_to_curve(c))
                .collect();
            
            // Copy grid_points (first 4 elements)
            let mut grid_points = [0u8; 4];
            for (i, &gp) in mdt.grid_points.iter().take(4).enumerate() {
                grid_points[i] = gp;
            }
            
            let matrix_channels = if matrix_curves.is_empty() { 0 } else { 3 };
            
            Some(ffi::A2B {
                input_curves,
                input_channels: mdt.num_input_channels as u32,
                grid_points,
                grid_data,
                is_16bit_grid,
                matrix_curves,
                matrix,
                matrix_bias,
                matrix_channels,
                output_curves,
                output_channels: mdt.num_output_channels as u32,
            })
        }
        LutWarehouse::Lut(_ldt) => {
            // TODO: Support legacy Lut8Type/Lut16Type (mft1/mft2) if needed
            // For now, only Multidimensional (mAB/mBA tags) are supported
            None
        }
    }
}

/// Convert moxcms ToneReprCurve to skcms-compatible TransferFunction.
/// Returns None if the curve cannot be represented as a parametric function.
fn convert_trc_to_transfer_function(
    trc: &moxcms::ToneReprCurve,
) -> Option<ffi::TransferFunction> {
    use moxcms::ToneReprCurve;

    match trc {
        ToneReprCurve::Parametric(params) => {
            // moxcms parametric curve: Vec<f32> with 7 parameters (g, a, b, c, d, e, f)
            // Matches skcms transfer function layout
            if params.len() >= 7 {
                Some(ffi::TransferFunction {
                    g: params[0],
                    a: params[1],
                    b: params[2],
                    c: params[3],
                    d: params[4],
                    e: params[5],
                    f: params[6],
                })
            } else {
                // Not enough parameters
                None
            }
        }
        ToneReprCurve::Lut(table) => {
            // Handle simple gamma curves (single-entry LUT)
            // In ICC, a single u16 value represents gamma in 8.8 fixed-point
            if table.len() == 1 {
                let gamma = table[0] as f32 / 256.0;
                // Simple power curve: f(x) = x^gamma
                Some(ffi::TransferFunction {
                    g: gamma,
                    a: 1.0,
                    b: 0.0,
                    c: 0.0,
                    d: 0.0,
                    e: 0.0,
                    f: 0.0,
                })
            } else {
                // Multi-entry table-based curves would need approximation
                // For now, we don't support this (C++ can use skcms with raw bytes)
                // TODO: Implement curve approximation if needed
                None
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Helper to create an empty IccProfile for testing
    fn empty_icc_profile() -> ffi::IccProfile {
        ffi::IccProfile {
            data_color_space: ffi::ColorSpace::Unknown,
            connection_space: ffi::ColorSpace::Unknown,
            rendering_intent: ffi::RenderingIntent::Perceptual,
            to_xyzd50: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            has_to_xyzd50: false,
            trc: [ffi::TransferFunction {
                g: 0.0,
                a: 0.0,
                b: 0.0,
                c: 0.0,
                d: 0.0,
                e: 0.0,
                f: 0.0,
            }; 3],
            has_trc: false,
            cicp: ffi::Cicp {
                color_primaries: 0,
                transfer_characteristics: 0,
                matrix_coefficients: 0,
                video_full_range_flag: 0,
            },
            has_cicp: false,
            a2b: empty_a2b(),
            has_a2b: false,
            b2a: empty_b2a(),
            has_b2a: false,
        }
    }

    fn empty_a2b() -> ffi::A2B {
        ffi::A2B {
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

    fn empty_b2a() -> ffi::B2A {
        ffi::B2A {
            input_curves: Vec::new(),
            input_channels: 0,
            matrix: ffi::Matrix3x3 {
                vals: [[0.0; 3]; 3],
            },
            matrix_bias: [0.0; 3],
            matrix_curves: Vec::new(),
            matrix_channels: 0,
            grid_points: [0; 4],
            grid_data: Vec::new(),
            is_16bit_grid: false,
            output_curves: Vec::new(),
            output_channels: 0,
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
        let mut data = vec![0u8; 128];
        // Put wrong signature at offset 36
        data[36..40].copy_from_slice(b"badd");

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(!result);
    }

    #[test]
    fn test_valid_signature_invalid_profile() {
        let mut data = vec![0u8; 128];
        // Put correct ICC signature "acsp" at offset 36
        data[36..40].copy_from_slice(b"acsp");

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        // This will fail moxcms parsing since it's not a real ICC profile
        assert!(!result);
    }

    #[test]
    fn test_valid_profile() {
        // Sample from AdobeRGB1998.icc - a real 560-byte ICC profile
        let data: [u8; 560] = [
            0x00, 0x00, 0x02, 0x30, 0x41, 0x44, 0x42, 0x45, 0x02, 0x10, 0x00, 0x00, 0x6d, 0x6e,
            0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20, 0x07, 0xd0, 0x00, 0x08,
            0x00, 0x0b, 0x00, 0x13, 0x00, 0x33, 0x00, 0x3b, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50,
            0x50, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6f, 0x6e, 0x65, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 0x41, 0x44, 0x42, 0x45,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x00, 0xfc,
            0x00, 0x00, 0x00, 0x32, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00,
            0x00, 0x6b, 0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x14,
            0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x14, 0x72, 0x54,
            0x52, 0x43, 0x00, 0x00, 0x01, 0xc4, 0x00, 0x00, 0x00, 0x0e, 0x67, 0x54, 0x52, 0x43,
            0x00, 0x00, 0x01, 0xd4, 0x00, 0x00, 0x00, 0x0e, 0x62, 0x54, 0x52, 0x43, 0x00, 0x00,
            0x01, 0xe4, 0x00, 0x00, 0x00, 0x0e, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xf4,
            0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
            0x00, 0x14, 0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x14,
            0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69,
            0x67, 0x68, 0x74, 0x20, 0x32, 0x30, 0x30, 0x30, 0x20, 0x41, 0x64, 0x6f, 0x62, 0x65,
            0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73, 0x20, 0x49, 0x6e, 0x63, 0x6f, 0x72,
            0x70, 0x6f, 0x72, 0x61, 0x74, 0x65, 0x64, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x41, 0x64, 0x6f, 0x62, 0x65, 0x20,
            0x52, 0x47, 0x42, 0x20, 0x28, 0x31, 0x39, 0x39, 0x38, 0x29, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x16, 0xcc, 0x58, 0x59,
            0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x18, 0x00, 0x00, 0x4f, 0xa5, 0x00, 0x00,
            0x04, 0xfc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x8d,
            0x00, 0x00, 0xa0, 0x2c, 0x00, 0x00, 0x0f, 0x95, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x26, 0x31, 0x00, 0x00, 0x10, 0x2f, 0x00, 0x00, 0xbe, 0x9c,
        ];

        let mut out = empty_icc_profile();
        let result = parse_icc_profile(&data, &mut out);
        assert!(result, "Failed to parse valid ICC profile");
        assert_eq!(out.data_color_space, ffi::ColorSpace::RGB);
        assert_eq!(out.connection_space, ffi::ColorSpace::XYZ);
        
        // AdobeRGB has toXYZD50 matrix and TRCs
        assert!(out.has_to_xyzd50, "AdobeRGB should have toXYZD50 matrix");
        assert!(out.has_trc, "AdobeRGB should have TRCs");
        
        // Verify the TRCs are gamma 2.2 (encoded as 563 in 8.8 fixed point = 2.199...)
        assert!(
            (out.trc[0].g - 2.2).abs() < 0.01,
            "Red TRC should be ~2.2, got {}",
            out.trc[0].g
        );
    }
}
