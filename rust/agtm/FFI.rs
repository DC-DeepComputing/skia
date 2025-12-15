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

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        // Reference the SkStreamAdapter type from skia_rust_common.
        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    // Rust functions, types, and methods that are exposed through FFI.
    //
    // To avoid duplication, there are no doc comments inside the `extern "Rust"`
    // section. The doc comments of these items can instead be found in the
    // actual Rust code, outside of the `#[cxx::bridge]` manifest.
    extern "Rust" {
        fn parse_agtm(
            input: UniquePtr<SkStreamAdapter>,
            output: &mut AgtmSyntax) -> bool;
    }
}

/// This provides a public C++ API for parsing AGTM metadata.
fn parse(
    input: cxx::UniquePtr<ffi::SkStreamAdapter>,
    output: &mut AgtmSyntax) -> bool {
    false
}

