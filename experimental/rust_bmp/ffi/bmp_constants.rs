// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! BMP Format Constants
//!
//! This module contains all magic numbers, signatures, and limits used
//! throughout the BMP decoder. Centralizing these constants ensures
//! consistency between detection and decoding logic.

// File Signatures
pub const BMP_SIGNATURE: [u8; 2] = [0x42, 0x4D];
pub const BMP_SIGNATURE_U16: u16 = 0x4D42;
pub const BITMAPARRAY_SIGNATURE: [u8; 2] = [0x42, 0x41];
pub const BMP_SIGNATURE_SIZE: usize = 2;

pub const JPEG_SOI_MARKER_1: u8 = 0xFF;
pub const JPEG_SOI_MARKER_2: u8 = 0xD8;
pub const JPEG_MIN_SIGNATURE_SIZE: usize = 2;

pub const PNG_SIGNATURE: [u8; 8] = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A];
pub const PNG_MIN_SIGNATURE_SIZE: usize = 8;

// Header Sizes
pub const BMP_FILE_HEADER_SIZE: usize = 14;
pub const MIN_INFO_HEADER_SIZE: usize = 40;
pub const MIN_FULL_VALIDATION_SIZE: usize = BMP_FILE_HEADER_SIZE + MIN_INFO_HEADER_SIZE; // 54

pub const MAX_DIMENSION: u32 = 1 << 16; // 65,536 - matches Chromium/Blink
pub const MAX_TOTAL_PIXELS: u64 = 100_000_000;

// Bit Depth Constants
pub const VALID_BITS_PER_PIXEL: &[u16] = &[1, 4, 8, 16, 24, 32];

// Compression Types
pub const COMPRESSION_RGB: u32 = 0;
pub const COMPRESSION_RLE8: u32 = 1;
pub const COMPRESSION_RLE4: u32 = 2;
pub const COMPRESSION_BITFIELDS: u32 = 3;
pub const COMPRESSION_JPEG: u32 = 4;
pub const COMPRESSION_PNG: u32 = 5;
pub const COMPRESSION_ALPHABITFIELDS: u32 = 6;

// Color Table Limits - Maximum palette entries
pub const PALETTE_ENTRY_SIZE: usize = 4;

// Pixel Format Constants
pub const BYTES_PER_PIXEL_RGB: usize = 3;
pub const BYTES_PER_PIXEL_RGBA: usize = 4;

/// Alpha values
pub const ALPHA_OPAQUE: u8 = 255;
pub const ALPHA_DEFAULT: u8 = 0xff;

// Row Alignment Constants
pub const ROW_ALIGNMENT_BITS: u32 = 32;
pub const ROW_ALIGNMENT_BYTES: usize = 4;

// Memory and Size Limits
pub const MAX_HEADER_SIZE: u32 = 1024;

// V4/V5 Header ICC Profile Fields

/// BITMAPV5HEADER size
pub const BITMAPV5HEADER_SIZE: u32 = 124;

/// Offset to ICC profile data offset (BITMAPV5HEADER, relative to info header start)
/// This is the csType field in V4/V5 headers, at offset 56
pub const OFFSET_CS_TYPE: usize = 56;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bmp_signature() {
        assert_eq!(BMP_SIGNATURE[0], 0x42); // 'B'
        assert_eq!(BMP_SIGNATURE[1], 0x4D); // 'M'
        assert_eq!(BMP_SIGNATURE_U16, 0x4D42);
    }

    #[test]
    fn test_bitmaparray_signature() {
        assert_eq!(BITMAPARRAY_SIGNATURE[0], 0x42); // 'B'
        assert_eq!(BITMAPARRAY_SIGNATURE[1], 0x41); // 'A'
    }

    #[test]
    fn test_jpeg_signature() {
        assert_eq!(JPEG_SOI_MARKER_1, 0xFF);
        assert_eq!(JPEG_SOI_MARKER_2, 0xD8);
    }

    #[test]
    fn test_png_signature() {
        assert_eq!(PNG_SIGNATURE[0], 0x89);
        assert_eq!(PNG_SIGNATURE[1], 0x50); // 'P'
        assert_eq!(PNG_SIGNATURE[2], 0x4E); // 'N'
        assert_eq!(PNG_SIGNATURE[3], 0x47); // 'G'
        assert_eq!(PNG_SIGNATURE.len(), 8);
    }

    #[test]
    fn test_header_sizes() {
        assert_eq!(BMP_FILE_HEADER_SIZE, 14);
        assert_eq!(MIN_INFO_HEADER_SIZE, 40);
        assert_eq!(MIN_FULL_VALIDATION_SIZE, 54);
    }

    #[test]
    fn test_max_dimension() {
        assert_eq!(MAX_DIMENSION, 65536);
        assert_eq!(MAX_DIMENSION, 1 << 16);
    }

    #[test]
    fn test_valid_bits_per_pixel() {
        assert_eq!(VALID_BITS_PER_PIXEL.len(), 6);
        assert!(VALID_BITS_PER_PIXEL.contains(&1));
        assert!(VALID_BITS_PER_PIXEL.contains(&4));
        assert!(VALID_BITS_PER_PIXEL.contains(&8));
        assert!(VALID_BITS_PER_PIXEL.contains(&16));
        assert!(VALID_BITS_PER_PIXEL.contains(&24));
        assert!(VALID_BITS_PER_PIXEL.contains(&32));
        assert!(!VALID_BITS_PER_PIXEL.contains(&2));
        assert!(!VALID_BITS_PER_PIXEL.contains(&64));
    }

    #[test]
    fn test_compression_constants() {
        assert_eq!(COMPRESSION_RGB, 0);
        assert_eq!(COMPRESSION_RLE8, 1);
        assert_eq!(COMPRESSION_RLE4, 2);
        assert_eq!(COMPRESSION_BITFIELDS, 3);
        assert_eq!(COMPRESSION_JPEG, 4);
        assert_eq!(COMPRESSION_PNG, 5);
        assert_eq!(COMPRESSION_ALPHABITFIELDS, 6);
    }

    #[test]
    fn test_pixel_format_constants() {
        assert_eq!(BYTES_PER_PIXEL_RGB, 3);
        assert_eq!(BYTES_PER_PIXEL_RGBA, 4);
        assert_eq!(ALPHA_OPAQUE, 255);
        assert_eq!(ALPHA_DEFAULT, 0xff);
    }

    #[test]
    fn test_row_alignment() {
        assert_eq!(ROW_ALIGNMENT_BITS, 32);
        assert_eq!(ROW_ALIGNMENT_BYTES, 4);
    }

    #[test]
    fn test_icc_profile_constants() {
        assert_eq!(BITMAPV5HEADER_SIZE, 124);
    }

    #[test]
    fn test_signature_sizes() {
        assert_eq!(BMP_SIGNATURE_SIZE, 2);
        assert_eq!(JPEG_MIN_SIGNATURE_SIZE, 2);
        assert_eq!(PNG_MIN_SIGNATURE_SIZE, 8);
    }

    #[test]
    fn test_max_total_pixels() {
        assert_eq!(MAX_TOTAL_PIXELS, 100_000_000);
    }
}

/// Offset to profile data offset (BITMAPV5HEADER, relative to info header start)
/// Field at offset 112 in V5 header
pub const OFFSET_PROFILE_DATA: usize = 112;

/// Offset to profile size (BITMAPV5HEADER, relative to info header start)
/// Field at offset 116 in V5 header
pub const OFFSET_PROFILE_SIZE: usize = 116;

/// Color space type value for embedded ICC profile (kProfileEmbedded)
/// Value: 0x4d424544 ("MBED" in ASCII)
pub const CS_TYPE_PROFILE_EMBEDDED: u32 = 0x4d424544;
