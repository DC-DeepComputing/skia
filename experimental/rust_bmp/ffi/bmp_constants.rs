// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! BMP Format Constants
//!
//! This module contains all magic numbers, signatures, and limits used
//! throughout the BMP decoder. Centralizing these constants ensures
//! consistency between detection and decoding logic.

// ============================================================================
// File Signatures
// ============================================================================

/// BMP file signature "BM" (0x42, 0x4D)
pub const BMP_SIGNATURE: [u8; 2] = [0x42, 0x4D];

/// BMP file signature as u16 (little-endian)
pub const BMP_SIGNATURE_U16: u16 = 0x4D42;

/// BITMAPARRAY signature "BA" (0x42, 0x41)
pub const BITMAPARRAY_SIGNATURE: [u8; 2] = [0x42, 0x41];

// ============================================================================
// Header Sizes
// ============================================================================

/// Minimum bytes needed for basic BMP signature detection
pub const MIN_SIGNATURE_SIZE: usize = 2;

/// Size of BMP file header (before info header)
pub const BMP_FILE_HEADER_SIZE: usize = 14;

/// Minimum info header size (BITMAPINFOHEADER)
pub const MIN_INFO_HEADER_SIZE: usize = 40;

/// Size for full BMP validation (file header + info header)
pub const MIN_FULL_VALIDATION_SIZE: usize = BMP_FILE_HEADER_SIZE + MIN_INFO_HEADER_SIZE; // 54

/// Minimum size for BITMAPARRAY format
pub const MIN_BITMAPARRAY_SIZE: usize = BMP_FILE_HEADER_SIZE + MIN_FULL_VALIDATION_SIZE;

/// Valid BMP header sizes
/// - 12: BITMAPCOREHEADER (OS/2 1.x)
/// - 40: BITMAPINFOHEADER (Windows 3.x+)
/// - 52: BITMAPV2INFOHEADER (undocumented)
/// - 56: BITMAPV3INFOHEADER (undocumented)
/// - 108: BITMAPV4HEADER (Windows 95+)
/// - 124: BITMAPV5HEADER (Windows 98+)
pub const VALID_HEADER_SIZES: &[u32] = &[12, 40, 52, 56, 108, 124];

// ============================================================================
// Dimension Limits
// ============================================================================

/// Maximum dimension for width or height
/// Uses 2 billion to match Chromium's validation (2,000,000,000)
/// This is well under i32::MAX (2,147,483,647) and u32::MAX
pub const MAX_DIMENSION: u32 = 2_000_000_000;

/// Maximum total pixels (width × height) to prevent memory exhaustion
/// Set to 100 million pixels as a realistic limit
pub const MAX_TOTAL_PIXELS: u64 = 100_000_000;

/// Maximum reasonable dimension for overflow checks during detection
/// Used for early rejection of obviously corrupt files
#[allow(dead_code)]
pub const MAX_DIMENSION_OVERFLOW_CHECK: u64 = 1 << 29; // ~536 million

// ============================================================================
// Bit Depth Constants
// ============================================================================

/// Valid bits per pixel values for BMP format
pub const VALID_BITS_PER_PIXEL: &[u16] = &[1, 4, 8, 16, 24, 32];

// ============================================================================
// Compression Types
// ============================================================================

/// Compression type: None / RGB (uncompressed)
pub const COMPRESSION_RGB: u32 = 0;

/// Compression type: RLE 8-bit
pub const COMPRESSION_RLE8: u32 = 1;

/// Compression type: RLE 4-bit
pub const COMPRESSION_RLE4: u32 = 2;

/// Compression type: Bitfields
pub const COMPRESSION_BITFIELDS: u32 = 3;

/// Compression type: JPEG (embedded)
pub const COMPRESSION_JPEG: u32 = 4;

/// Compression type: PNG (embedded)
pub const COMPRESSION_PNG: u32 = 5;

/// Compression type: Alpha Bitfields (Windows CE)
#[allow(dead_code)]
pub const COMPRESSION_ALPHABITFIELDS: u32 = 6;

/// Maximum valid compression type value
pub const MAX_COMPRESSION_TYPE: u32 = 6;

// ============================================================================
// Color Table Limits
// ============================================================================

/// Maximum number of palette entries for 1-bit images
pub const MAX_PALETTE_1BPP: u32 = 2;

/// Maximum number of palette entries for 4-bit images
pub const MAX_PALETTE_4BPP: u32 = 16;

/// Maximum number of palette entries for 8-bit images
pub const MAX_PALETTE_8BPP: u32 = 256;

/// Size of each palette entry in bytes (BGRA format)
pub const PALETTE_ENTRY_SIZE: usize = 4;

// ============================================================================
// Pixel Format Constants
// ============================================================================

/// Bytes per pixel for RGB output format
pub const BYTES_PER_PIXEL_RGB: usize = 3;

/// Bytes per pixel for RGBA output format
pub const BYTES_PER_PIXEL_RGBA: usize = 4;

/// Opaque alpha value (fully opaque)
pub const ALPHA_OPAQUE: u8 = 255;

/// Default alpha value for missing alpha channel
pub const ALPHA_DEFAULT: u8 = 0xff;

// ============================================================================
// Row Alignment Constants
// ============================================================================

/// BMP rows are aligned to 32-bit (4-byte) boundaries
pub const ROW_ALIGNMENT_BITS: u32 = 32;

/// BMP row alignment in bytes
pub const ROW_ALIGNMENT_BYTES: usize = 4;

/// Divisor for calculating aligned row size (bits to DWORD)
pub const ROW_ALIGNMENT_DIVISOR: u32 = 32;

// ============================================================================
// Embedded Format Signatures
// ============================================================================

/// JPEG Start of Image (SOI) marker - first byte
pub const JPEG_SOI_MARKER_1: u8 = 0xFF;

/// JPEG Start of Image (SOI) marker - second byte
pub const JPEG_SOI_MARKER_2: u8 = 0xD8;

/// PNG file signature (8 bytes): 89 50 4E 47 0D 0A 1A 0A
pub const PNG_SIGNATURE: [u8; 8] = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A];

/// Minimum bytes needed to detect JPEG signature (0xFF 0xD8)
pub const JPEG_MIN_SIGNATURE_SIZE: usize = 2;

/// Minimum bytes needed to detect PNG signature
pub const PNG_MIN_SIGNATURE_SIZE: usize = 8;

// ============================================================================
// Memory and Size Limits
// ============================================================================

/// Maximum header size allowed (prevent excessive allocation)
pub const MAX_HEADER_SIZE: u32 = 1024;

/// Maximum decoded image size (128 MB)
#[allow(dead_code)]
pub const MAX_DECODED_BYTES: usize = 128 * 1024 * 1024;

// ============================================================================
// Field Offsets (within BMP file header, starting at 0)
// ============================================================================

/// Offset to file signature
#[allow(dead_code)]
pub const OFFSET_SIGNATURE: usize = 0;

/// Offset to file size field
#[allow(dead_code)]
pub const OFFSET_FILE_SIZE: usize = 2;

/// Offset to pixel data offset field
#[allow(dead_code)]
pub const OFFSET_DATA_OFFSET: usize = 10;

// ============================================================================
// Field Offsets (within info header, starting at BMP_FILE_HEADER_SIZE)
// ============================================================================

/// Offset to header size field (relative to info header start)
#[allow(dead_code)]
pub const OFFSET_HEADER_SIZE: usize = 0;

/// Offset to width field (relative to info header start)
pub const OFFSET_WIDTH: usize = 4;

/// Offset to height field (relative to info header start)
#[allow(dead_code)]
pub const OFFSET_HEIGHT: usize = 8;

/// Offset to planes field (relative to info header start)
#[allow(dead_code)]
pub const OFFSET_PLANES: usize = 12;

/// Offset to bits per pixel field (relative to info header start)
pub const OFFSET_BITS_PER_PIXEL: usize = 14;

/// Offset to compression field (relative to info header start)
pub const OFFSET_COMPRESSION: usize = 16;

/// Offset to image size field (relative to info header start)
#[allow(dead_code)]
pub const OFFSET_IMAGE_SIZE: usize = 20;

/// Offset to colors used field (relative to info header start)
pub const OFFSET_COLORS_USED: usize = 32;

/// Offset to colors important field (relative to info header start)
pub const OFFSET_COLORS_IMPORTANT: usize = 36;

// ============================================================================
// V4/V5 Header ICC Profile Fields
// ============================================================================

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
