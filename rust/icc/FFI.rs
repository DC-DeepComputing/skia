// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC profile parser FFI bindings.
//!
//! Parsing happens in checked Rust code, then the owned result is exposed
//! through skcms-compatible bridge structures for color transformations.

mod icc_curve;
mod icc_lut;
mod icc_parser;
mod icc_reader;

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    /// Color space types (matches skcms_Signature enum values).
    #[repr(u32)]
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum skcms_Signature {
        skcms_Signature_CMYK = 0x434D594B,
        skcms_Signature_Gray = 0x47524159,
        skcms_Signature_RGB = 0x52474220,
        skcms_Signature_Lab = 0x4C616220,
        skcms_Signature_XYZ = 0x58595A20,
        skcms_Signature_CIELUV = 0x4C757620,
        skcms_Signature_YCbCr = 0x59436272,
        skcms_Signature_CIEYxy = 0x59787920,
        skcms_Signature_HSV = 0x48535620,
        skcms_Signature_HLS = 0x484C5320,
        skcms_Signature_CMY = 0x434D5920,
        skcms_Signature_2CLR = 0x32434C52,
        skcms_Signature_3CLR = 0x33434C52,
        skcms_Signature_4CLR = 0x34434C52,
        skcms_Signature_5CLR = 0x35434C52,
        skcms_Signature_6CLR = 0x36434C52,
        skcms_Signature_7CLR = 0x37434C52,
        skcms_Signature_8CLR = 0x38434C52,
        skcms_Signature_9CLR = 0x39434C52,
        skcms_Signature_10CLR = 0x41434C52,
        skcms_Signature_11CLR = 0x42434C52,
        skcms_Signature_12CLR = 0x43434C52,
        skcms_Signature_13CLR = 0x44434C52,
        skcms_Signature_14CLR = 0x45434C52,
        skcms_Signature_15CLR = 0x46434C52,
    }

    extern "C++" {
        include!("modules/skcms/skcms.h");
        include!("rust/icc/FFI.h");

        #[namespace = ""]
        type skcms_Signature;
    }

    // These definitions mirror the public skcms structs. FFI.cpp has layout
    // assertions, and RustIccTest.cpp verifies the known fields end to end.
    struct Matrix3x3 {
        vals: [[f32; 3]; 3],
    }

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

    #[derive(Clone, Copy)]
    struct Cicp {
        color_primaries: u8,
        transfer_characteristics: u8,
        matrix_coefficients: u8,
        video_full_range_flag: u8,
    }

    struct Curve {
        table_entries: u32,
        parametric: TransferFunction,
        table_data: Vec<u8>,
    }

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

    struct B2A {
        input_curves: Vec<Curve>,
        input_channels: u32,
        matrix: Matrix3x3,
        matrix_bias: [f32; 3],
        matrix_curves: Vec<Curve>,
        matrix_channels: u32,
        grid_points: [u8; 4],
        grid_data: Vec<u8>,
        is_16bit_grid: bool,
        output_curves: Vec<Curve>,
        output_channels: u32,
    }

    struct IccProfile {
        data_color_space: skcms_Signature,
        connection_space: skcms_Signature,
        to_xyzd50: Matrix3x3,
        has_to_xyzd50: bool,
        trc_r: Curve,
        trc_g: Curve,
        trc_b: Curve,
        has_trc: bool,
        cicp: Cicp,
        has_cicp: bool,
        a2b: A2B,
        has_a2b: bool,
        b2a: B2A,
        has_b2a: bool,
    }

    extern "Rust" {
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}

pub fn parse_icc_profile(data: &[u8], out: &mut ffi::IccProfile) -> bool {
    let Some(profile) = icc_parser::parse_profile(data) else {
        return false;
    };
    *out = profile;
    true
}