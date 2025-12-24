// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides a C++ interface to parse AGTM metadata in Rust.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

#[cxx::bridge(namespace = "rust_agtm")]
mod ffi {
    /// FFI/layering-friendly equivalent of `skhdr::AgtmSyntax` from C/C++.
    struct AgtmSyntax {
        // syntax elements of smpte_st_2094_50_application_info()
        application_version: u8,

        // syntax elements of smpte_st_2094_50_color_volume_transform()
        has_custom_hdr_reference_white_flag: bool,
        has_adaptive_tone_map_flag: bool,
        hdr_reference_white: u16,

        // syntax elements of smpte_st_2094_50_adaptive_tone_map()
        baseline_hdr_headroom: u16,
        use_reference_white_tone_mapping_flag: bool,
        num_alternate_images: u8,
        gain_application_space_chromaticities_flag: u8,
        has_common_component_mix_params_flag: bool,
        has_common_curve_params_flag: bool,
        gain_application_space_chromaticities: [u16; 8],
        alternate_hdr_headrooms: [u16; 4],

        // syntax elements of smpte_st_2094_50_component_mixing()
        component_mixing_type: [u8; 4],
        has_component_mixing_coefficient_flag: [[bool; 6]; 4],
        component_mixing_coefficient: [[u16; 6]; 4],

        // syntax elements of smpte_st_2094_50_gain_curve()
        gain_curve_num_control_points_minus_1: [u8; 4],
        gain_curve_use_pchip_slope_flag: [bool; 4],
        gain_curve_control_points_x: [[u16; 32]; 4],
        gain_curve_control_points_y: [[u16; 32]; 4],
        gain_curve_control_points_theta: [[u16; 32]; 4],
    }

    // Parse the bytes of AGTM metadata in `input` and write the result to
    // `output`. Return true on success, otherwise return false.
    extern "Rust" {
        fn parse_agtm(
            input: &[u8],
            output: &mut AgtmSyntax) -> bool;
    }
}

use ffi::AgtmSyntax;

/// This provides a public C++ API for parsing AGTM metadata.
fn parse_agtm(
    input: &[u8],
    output: &mut AgtmSyntax) -> bool {
    // TODO(https://issues.skia.org/37711): Actually implement something!
    let _ = (input, output);
    false
}

