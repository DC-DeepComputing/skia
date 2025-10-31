// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Type definitions for BMP decoder
//!
//! Note: ColorType and AlphaType are defined in FFI.rs to avoid conflicts
//! This module contains only BMP-specific internal types.

use thiserror::Error;

/// Core BMP decoder error types
#[derive(Debug, Error)]
pub enum BmpError {
    #[error("Insufficient data available")]
    InsufficientData,

    #[error("Invalid data: {0}")]
    InvalidData(String),

    #[error("Unsupported feature: {0}")]
    UnsupportedFeature(String),

    #[error("Invalid file type signature")]
    InvalidFileType,

    #[error("Header too small: got {got}, expected at least {expected}")]
    UnsupportedHeaderSize { got: u32, expected: u32 },

    #[error("Invalid header data")]
    InvalidHeader,

    #[error("JPEG decoding failed: {0}")]
    JpegDecodingFailed(String),

    #[error("PNG decoding failed: {0}")]
    PngDecodingFailed(String),

    #[error("Invalid data offset")]
    InvalidDataOffset,

    #[error("RLE decoding failed")]
    RleDecodingFailed,
}

/// BMP compression types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CompressionType {
    None,
    Rle8,
    Rle4,
    Jpeg,
    Png,
    BitFields,
}

impl CompressionType {
    #[cfg(test)]
    pub fn is_valid(value: u32) -> bool {
        value <= 6 // COMPRESSION_ALPHABITFIELDS is the maximum
    }
}

/// BMP header information combining file and info headers (matches Chromium's approach)
#[derive(Debug, Clone)]
pub struct BmpHeaderInfo {
    pub width: u32,
    pub height: u32,
    pub bits_per_pixel: u16,
    pub compression: CompressionType,
    pub colors_used: u32,
    pub data_offset: u32,
    pub file_size: u32,
    pub icc_profile_data: Option<u32>,
    pub icc_profile_size: Option<u32>,
}

/// Color masks for BITFIELDS compression
#[derive(Debug, Clone, Copy)]
pub struct ColorMasks {
    pub red_mask: u32,
    pub green_mask: u32,
    pub blue_mask: u32,
    pub alpha_mask: u32,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compression_type_is_valid() {
        assert!(CompressionType::is_valid(0)); // RGB
        assert!(CompressionType::is_valid(1)); // RLE8
        assert!(CompressionType::is_valid(2)); // RLE4
        assert!(CompressionType::is_valid(3)); // BITFIELDS
        assert!(CompressionType::is_valid(4)); // JPEG
        assert!(CompressionType::is_valid(5)); // PNG
        assert!(CompressionType::is_valid(6)); // ALPHABITFIELDS
        assert!(!CompressionType::is_valid(7)); // Invalid
        assert!(!CompressionType::is_valid(100)); // Invalid
    }

    #[test]
    fn test_compression_type_equality() {
        assert_eq!(CompressionType::None, CompressionType::None);
        assert_eq!(CompressionType::Rle8, CompressionType::Rle8);
        assert_ne!(CompressionType::Rle8, CompressionType::Rle4);
        assert_ne!(CompressionType::Jpeg, CompressionType::Png);
    }

    #[test]
    fn test_bmp_error_display() {
        let err = BmpError::InsufficientData;
        assert_eq!(err.to_string(), "Insufficient data available");

        let err = BmpError::InvalidData("test message".to_string());
        assert_eq!(err.to_string(), "Invalid data: test message");

        let err = BmpError::UnsupportedHeaderSize {
            got: 10,
            expected: 40,
        };
        assert_eq!(
            err.to_string(),
            "Header too small: got 10, expected at least 40"
        );
    }

    #[test]
    fn test_bmp_header_info_creation() {
        let header = BmpHeaderInfo {
            width: 640,
            height: 480,
            bits_per_pixel: 24,
            compression: CompressionType::None,
            colors_used: 0,
            data_offset: 54,
            file_size: 1000,
            icc_profile_data: None,
            icc_profile_size: None,
        };

        assert_eq!(header.width, 640);
        assert_eq!(header.height, 480);
        assert_eq!(header.bits_per_pixel, 24);
        assert_eq!(header.compression, CompressionType::None);
        assert_eq!(header.data_offset, 54);
    }

    #[test]
    fn test_bmp_header_info_with_icc() {
        let header = BmpHeaderInfo {
            width: 100,
            height: 100,
            bits_per_pixel: 32,
            compression: CompressionType::BitFields,
            colors_used: 0,
            data_offset: 138,
            file_size: 50000,
            icc_profile_data: Some(1000),
            icc_profile_size: Some(500),
        };

        assert!(header.icc_profile_data.is_some());
        assert_eq!(header.icc_profile_data.unwrap(), 1000);
        assert!(header.icc_profile_size.is_some());
        assert_eq!(header.icc_profile_size.unwrap(), 500);
    }

    #[test]
    fn test_color_masks() {
        let masks = ColorMasks {
            red_mask: 0x00FF0000,
            green_mask: 0x0000FF00,
            blue_mask: 0x000000FF,
            alpha_mask: 0xFF000000,
        };

        assert_eq!(masks.red_mask, 0x00FF0000);
        assert_eq!(masks.green_mask, 0x0000FF00);
        assert_eq!(masks.blue_mask, 0x000000FF);
        assert_eq!(masks.alpha_mask, 0xFF000000);
    }

    #[test]
    fn test_color_masks_rgb565() {
        let masks = ColorMasks {
            red_mask: 0xF800,   // 5 bits
            green_mask: 0x07E0, // 6 bits
            blue_mask: 0x001F,  // 5 bits
            alpha_mask: 0,      // No alpha
        };

        assert_eq!(masks.red_mask, 0xF800);
        assert_eq!(masks.green_mask, 0x07E0);
        assert_eq!(masks.blue_mask, 0x001F);
        assert_eq!(masks.alpha_mask, 0);
    }
}
