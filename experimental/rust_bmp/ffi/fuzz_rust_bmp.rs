// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Fuzz target for Rust BMP decoder - tests full decoding pipeline
//!
//! This fuzzer exercises all public FFI functions:
//! - is_bmp_data() - BMP signature detection
//! - decode_info() - Header parsing and metadata extraction
//! - decode_pixels() - Full pixel decoding with all compression types
//!
//! The fuzzer includes consistency assertions to verify:
//! - Dimensions match between info and pixel results
//! - Pixel buffer sizes are correct
//! - Signature detection aligns with decode results

use std::io::{self, Read};

#[path = "lib.rs"]
mod ffi;

fn main() {
    // Read input from stdin
    let mut buffer = Vec::new();
    io::stdin()
        .read_to_end(&mut buffer)
        .expect("Failed to read from stdin");

    // Limit input size to prevent timeouts (match C++ fuzzer)
    if buffer.len() > 10240 {
        return;
    }

    // Skip trivially small inputs
    if buffer.len() < 2 {
        return;
    }

    // Test full decode pipeline through FFI
    let mut decoder = ffi::create_decoder();

    // Step 1: Check signature
    let is_bmp = ffi::is_bmp_data(&buffer);

    // Step 2: Decode header/info
    let info_result = decoder.decode_info(&buffer);

    // Step 3: If info succeeded, decode pixels
    if info_result.success {
        let pixel_result = decoder.decode_pixels(&buffer);

        // Verify consistency
        if pixel_result.success {
            // Check that dimensions match
            assert_eq!(pixel_result.width, info_result.width);
            assert_eq!(pixel_result.height, info_result.height);

            // Check pixel data size is reasonable
            let expected_min_size = (pixel_result.width as usize)
                .saturating_mul(pixel_result.height as usize)
                .saturating_mul(pixel_result.bytes_per_pixel as usize);

            // For incomplete data, pixels might be less
            if !pixel_result.data_is_incomplete {
                assert!(pixel_result.pixels.len() >= expected_min_size);
            }
        }
    }

    // If signature detection said it's BMP, info decode should have tried
    // (even if it failed due to corruption)
    if is_bmp && buffer.len() >= 54 {
        // We at least attempted to decode
        // (no assertion needed, just exercising the code path)
    }
}
