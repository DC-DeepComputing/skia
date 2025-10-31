// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! ICC Color Profile Processing using moxcms
//!
//! This module handles ICC color profile parsing and color transformation
//! for BMP images with embedded ICC profiles (V4/V5 headers).
//!
//! Uses moxcms (pure Rust) for memory-safe ICC parsing, complying with
//! Chromium's Rule of 2 security requirements.

use super::bmp_constants::BYTES_PER_PIXEL_RGB;
use moxcms::{ColorProfile, Layout, TransformOptions};

/// Extract and parse ICC profile from BMP data
pub fn extract_icc_profile(
    data: &[u8],
    profile_offset: u32,
    profile_size: u32,
) -> Option<ColorProfile> {
    let offset = profile_offset as usize;
    let size = profile_size as usize;

    if offset >= data.len() || size == 0 {
        return None;
    }

    let end = offset.checked_add(size)?;
    if end > data.len() {
        return None;
    }

    let icc_data = &data[offset..end];

    ColorProfile::new_from_slice(icc_data).ok()
}

/// Apply ICC color transform to RGB pixels
///
/// Transforms pixels from the source ICC profile color space to sRGB.
/// Returns transformed pixels, or None if transformation fails.
pub fn apply_icc_transform(
    profile: &ColorProfile,
    pixels: &[u8],
    width: u32,
    height: u32,
) -> Option<Vec<u8>> {
    let srgb = ColorProfile::new_srgb();

    let transform = profile
        .create_transform_8bit(Layout::Rgb, &srgb, Layout::Rgb, TransformOptions::default())
        .ok()?;

    let pixel_count = (width as usize) * (height as usize);
    let expected_size = pixel_count * BYTES_PER_PIXEL_RGB;

    if pixels.len() != expected_size {
        return None;
    }

    let mut transformed = vec![0u8; expected_size];

    match transform.transform(pixels, &mut transformed) {
        Ok(_) => Some(transformed),
        Err(_) => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_extract_icc_profile_invalid_offset() {
        let data = vec![0u8; 100];

        // Offset beyond data
        assert!(extract_icc_profile(&data, 200, 10).is_none());

        // Zero size
        assert!(extract_icc_profile(&data, 0, 0).is_none());
    }

    #[test]
    fn test_extract_icc_profile_invalid_size() {
        let data = vec![0u8; 100];

        // Size extends beyond data
        assert!(extract_icc_profile(&data, 50, 100).is_none());
    }

    #[test]
    fn test_extract_icc_profile_empty_data() {
        let data: Vec<u8> = vec![];
        assert!(extract_icc_profile(&data, 0, 10).is_none());
    }

    #[test]
    fn test_apply_icc_transform_wrong_pixel_count() {
        let srgb = ColorProfile::new_srgb();

        // 2x2 image should have 12 bytes (4 pixels * 3 bytes)
        // But we provide only 6 bytes
        let pixels = vec![255u8; 6];
        let result = apply_icc_transform(&srgb, &pixels, 2, 2);

        assert!(result.is_none());
    }

    #[test]
    fn test_apply_icc_transform_empty_pixels() {
        let srgb = ColorProfile::new_srgb();
        let pixels: Vec<u8> = vec![];

        // 0x0 image should work with empty pixels
        let result = apply_icc_transform(&srgb, &pixels, 0, 0);
        assert!(result.is_some());
        assert_eq!(result.unwrap().len(), 0);
    }

    #[test]
    fn test_apply_icc_transform_srgb_identity() {
        let srgb = ColorProfile::new_srgb();

        // 1x1 red pixel
        let pixels = vec![255, 0, 0];
        let result = apply_icc_transform(&srgb, &pixels, 1, 1);

        // sRGB to sRGB should be approximately identity
        assert!(result.is_some());
        let transformed = result.unwrap();
        assert_eq!(transformed.len(), 3);

        // Red channel should remain roughly the same (allowing for rounding)
        assert!(transformed[0] >= 250);
    }

    #[test]
    fn test_apply_icc_transform_multiple_pixels() {
        let srgb = ColorProfile::new_srgb();

        // 2x2 image with different colors
        let pixels = vec![
            255, 0, 0, // Red
            0, 255, 0, // Green
            0, 0, 255, // Blue
            255, 255, 255, // White
        ];

        let result = apply_icc_transform(&srgb, &pixels, 2, 2);
        assert!(result.is_some());

        let transformed = result.unwrap();
        assert_eq!(transformed.len(), 12); // 4 pixels * 3 bytes
    }

    #[test]
    fn test_srgb_profile_creation() {
        // Test that we can create sRGB profile
        let srgb = ColorProfile::new_srgb();

        // Should be able to create a transform with it
        let transform_result = srgb.create_transform_8bit(
            Layout::Rgb,
            &srgb,
            Layout::Rgb,
            TransformOptions::default(),
        );

        assert!(transform_result.is_ok());
    }
}
