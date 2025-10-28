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
pub const MIN_BITMAPARRAY_SIZE: usize = BMP_FILE_HEADER_SIZE + MIN_FULL_VALIDATION_SIZE;

/// Valid BMP header sizes
/// - 12: BITMAPCOREHEADER (OS/2 1.x)
/// - 40: BITMAPINFOHEADER (Windows 3.x+)
/// - 52: BITMAPV2INFOHEADER (undocumented)
/// - 56: BITMAPV3INFOHEADER (undocumented)
/// - 108: BITMAPV4HEADER (Windows 95+)
/// - 124: BITMAPV5HEADER (Windows 98+)
pub const VALID_HEADER_SIZES: &[u32] = &[12, 40, 52, 56, 108, 124];

pub const MAX_DIMENSION: u32 = 1 << 16; // 65,536 - matches Chromium/Skia
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
pub const MAX_COMPRESSION_TYPE: u32 = 6;

// Color Table Limits - Maximum palette entries
pub const MAX_PALETTE_1BPP: u32 = 1 << 1;
pub const MAX_PALETTE_4BPP: u32 = 1 << 4;
pub const MAX_PALETTE_8BPP: u32 = 1 << 8;
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

// Field Offsets (within info header, starting at BMP_FILE_HEADER_SIZE)

/// Offset to width field (relative to info header start)
pub const OFFSET_WIDTH: usize = 4;

/// Offset to bits per pixel field (relative to info header start)
pub const OFFSET_BITS_PER_PIXEL: usize = 14;

/// Offset to compression field (relative to info header start)
pub const OFFSET_COMPRESSION: usize = 16;

/// Offset to colors used field (relative to info header start)
pub const OFFSET_COLORS_USED: usize = 32;

/// Offset to colors important field (relative to info header start)
pub const OFFSET_COLORS_IMPORTANT: usize = 36;

// V4/V5 Header ICC Profile Fields

/// BITMAPV5HEADER size
pub const BITMAPV5HEADER_SIZE: u32 = 124;

/// Offset to ICC profile data offset (BITMAPV5HEADER, relative to info header start)
/// This is the csType field in V4/V5 headers, at offset 56
pub const OFFSET_CS_TYPE: usize = 56;

/// Offset to profile data offset (BITMAPV5HEADER, relative to info header start)
/// Field at offset 112 in V5 header
pub const OFFSET_PROFILE_DATA: usize = 112;

/// Offset to profile size (BITMAPV5HEADER, relative to info header start)
/// Field at offset 116 in V5 header
pub const OFFSET_PROFILE_SIZE: usize = 116;

/// Color space type value for embedded ICC profile (kProfileEmbedded)
/// Value: 0x4d424544 ("MBED" in ASCII)
pub const CS_TYPE_PROFILE_EMBEDDED: u32 = 0x4d424544;
