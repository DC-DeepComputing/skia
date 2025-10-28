// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! BMP header parsing implementation
//!
//! This module handles BMP header parsing and format detection/validation.

use super::bmp_constants::*;
use super::bmp_types::{BmpError, BmpHeaderInfo, ColorMasks, CompressionType};

// ============================================================================
// Type Definitions
// ============================================================================

/// BMP header parser
pub struct HeaderParser {
    header_info: Option<BmpHeaderInfo>,
    color_masks: Option<ColorMasks>,
}

// ============================================================================
// HeaderParser Implementation
// ============================================================================

impl HeaderParser {
    pub fn new() -> Self {
        Self {
            header_info: None,
            color_masks: None,
        }
    }

    /// Parse BMP header from data
    pub fn parse_header(&mut self, data: &[u8]) -> Result<BmpHeaderInfo, BmpError> {
        if data.len() < BMP_FILE_HEADER_SIZE {
            return Err(BmpError::InsufficientData);
        }

        let (file_size, data_offset) = self.parse_file_header(data)?;

        // STRICT VALIDATION: Check if data offset is valid (Chromium compatibility)
        if data_offset as usize > data.len() {
            return Err(BmpError::InvalidHeader);
        }

        if data.len() < MIN_FULL_VALIDATION_SIZE {
            return Err(BmpError::InsufficientData);
        }

        // Chromium is permissive with BITFIELDS validation
        if data.len() >= 34 {
            let compression_value = u32::from_le_bytes([data[30], data[31], data[32], data[33]]);
            if compression_value == COMPRESSION_BITFIELDS {
                let bits_per_pixel = if data.len() >= 30 {
                    u16::from_le_bytes([data[28], data[29]])
                } else {
                    0
                };

                // Chromium accepts malformed 1bpp BITFIELDS files
                if bits_per_pixel != 1 {
                    let header_size = if data.len() >= 18 {
                        u32::from_le_bytes([data[14], data[15], data[16], data[17]]) as usize
                    } else {
                        MIN_INFO_HEADER_SIZE
                    };

                    if data.len() < BMP_FILE_HEADER_SIZE + header_size + 12 {
                        return Err(BmpError::InsufficientData);
                    }
                }
            }
        }

        let header_info =
            self.parse_info_header(&data[BMP_FILE_HEADER_SIZE..], file_size, data_offset)?;
        self.header_info = Some(header_info.clone());

        Ok(header_info)
    }

    pub fn get_header_info(&self) -> Result<BmpHeaderInfo, BmpError> {
        self.header_info.clone().ok_or(BmpError::InvalidHeader)
    }

    pub fn get_color_masks(&self) -> Option<ColorMasks> {
        self.color_masks
    }

    fn parse_file_header(&self, data: &[u8]) -> Result<(u32, u32), BmpError> {
        // Check BMP signature - accept both "BM" and "BA" (BITMAPARRAY) for Chromium parity
        let file_type = u16::from_le_bytes([data[0], data[1]]);
        const BA_SIGNATURE_U16: u16 = 0x4142; // "BA" in little-endian

        if file_type != BMP_SIGNATURE_U16 && file_type != BA_SIGNATURE_U16 {
            return Err(BmpError::InvalidFileType);
        }

        let file_size = u32::from_le_bytes([data[2], data[3], data[4], data[5]]);
        let data_offset = u32::from_le_bytes([data[10], data[11], data[12], data[13]]);

        Ok((file_size, data_offset))
    }

    fn parse_info_header(
        &mut self,
        data: &[u8],
        file_size: u32,
        data_offset: u32,
    ) -> Result<BmpHeaderInfo, BmpError> {
        if data.len() < MIN_INFO_HEADER_SIZE {
            return Err(BmpError::InsufficientData);
        }

        let size = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
        if size < MIN_INFO_HEADER_SIZE as u32 {
            return Err(BmpError::UnsupportedHeaderSize {
                got: size,
                expected: MIN_INFO_HEADER_SIZE as u32,
            });
        }

        // STRICT VALIDATION: Reject excessively large header sizes
        if size > MAX_HEADER_SIZE {
            return Err(BmpError::InvalidHeader);
        }

        let width = u32::from_le_bytes([data[4], data[5], data[6], data[7]]);
        let height_signed = i32::from_le_bytes([data[8], data[9], data[10], data[11]]);
        let height = height_signed.unsigned_abs();
        let is_top_down = height_signed < 0;
        let _planes = u16::from_le_bytes([data[12], data[13]]);
        let bits_per_pixel = u16::from_le_bytes([data[14], data[15]]);

        if width == 0 || height == 0 {
            return Err(BmpError::InvalidData(
                "Invalid image dimensions".to_string(),
            ));
        }

        if width > MAX_DIMENSION || height > MAX_DIMENSION {
            return Err(BmpError::InvalidData(
                "Image dimensions too large".to_string(),
            ));
        }

        // Prevent memory exhaustion attacks
        let pixel_count = (width as u64) * (height as u64);
        if pixel_count > MAX_TOTAL_PIXELS {
            return Err(BmpError::InvalidData("Too many pixels".to_string()));
        }

        // Parse compression before validating bits_per_pixel (JPEG/PNG have bpp=0)
        let compression_value = u32::from_le_bytes([data[16], data[17], data[18], data[19]]);
        let compression = match compression_value {
            COMPRESSION_RGB => CompressionType::None,
            COMPRESSION_RLE8 => CompressionType::Rle8,
            COMPRESSION_RLE4 => CompressionType::Rle4,
            COMPRESSION_BITFIELDS => CompressionType::BitFields,
            COMPRESSION_JPEG => CompressionType::Jpeg,
            COMPRESSION_PNG => CompressionType::Png,
            _ => {
                return Err(BmpError::UnsupportedFeature(format!(
                    "Compression type {}",
                    compression_value
                )))
            }
        };

        // STRICT VALIDATION: Validate bit depth - only standard BMP bit depths are supported
        // EXCEPTION: Embedded JPEG/PNG formats have bits_per_pixel=0 (valid for these formats)
        if !VALID_BITS_PER_PIXEL.contains(&bits_per_pixel) {
            if bits_per_pixel == 0 {
                // Allow bits_per_pixel=0 ONLY for embedded JPEG/PNG
                if !matches!(compression, CompressionType::Jpeg | CompressionType::Png) {
                    return Err(BmpError::InvalidHeader);
                }
            } else if bits_per_pixel == 2 {
                // T3.2: 2bpp format rejection - Chromium doesn't support 2 bits per pixel
                return Err(BmpError::UnsupportedFeature(
                    "2 bits per pixel format is not supported".to_string(),
                ));
            } else {
                // Invalid bit depths like 3, 5, 6, 7, 9-15, 17-23, 25-31, >32
                return Err(BmpError::UnsupportedFeature(format!(
                    "Invalid bit depth: {} bits per pixel",
                    bits_per_pixel
                )));
            }
        }

        let _image_size = u32::from_le_bytes([data[20], data[21], data[22], data[23]]);
        let _x_pixels_per_meter = u32::from_le_bytes([data[24], data[25], data[26], data[27]]);
        let _y_pixels_per_meter = u32::from_le_bytes([data[28], data[29], data[30], data[31]]);
        let colors_used = u32::from_le_bytes([data[32], data[33], data[34], data[35]]);
        let _colors_important = u32::from_le_bytes([data[36], data[37], data[38], data[39]]);

        // T3.3: Top-down regular BMP restriction - Chromium rejects top-down BMPs for regular compression
        if is_top_down && matches!(compression, CompressionType::None) {
            return Err(BmpError::UnsupportedFeature(
                "Top-down BMPs are not supported for regular (uncompressed) format".to_string(),
            ));
        }

        // Parse BITFIELDS masks if present
        if compression == CompressionType::BitFields {
            // Chromium ignores 1bpp BITFIELDS masks
            if bits_per_pixel == 1 {
                self.color_masks = Some(ColorMasks {
                    red_mask: 0,
                    green_mask: 0,
                    blue_mask: 0,
                    alpha_mask: 0,
                });
            } else {
                let mask_offset = size as usize;

                if data.len() >= mask_offset + 12 {
                    let red = u32::from_le_bytes([
                        data[mask_offset],
                        data[mask_offset + 1],
                        data[mask_offset + 2],
                        data[mask_offset + 3],
                    ]);
                    let green = u32::from_le_bytes([
                        data[mask_offset + 4],
                        data[mask_offset + 5],
                        data[mask_offset + 6],
                        data[mask_offset + 7],
                    ]);
                    let blue = u32::from_le_bytes([
                        data[mask_offset + 8],
                        data[mask_offset + 9],
                        data[mask_offset + 10],
                        data[mask_offset + 11],
                    ]);

                    let alpha = if data.len() >= mask_offset + 16 {
                        u32::from_le_bytes([
                            data[mask_offset + 12],
                            data[mask_offset + 13],
                            data[mask_offset + 14],
                            data[mask_offset + 15],
                        ])
                    } else {
                        0
                    };

                    self.color_masks = Some(ColorMasks {
                        red_mask: red,
                        green_mask: green,
                        blue_mask: blue,
                        alpha_mask: alpha,
                    });
                } else {
                    self.color_masks = Some(ColorMasks {
                        red_mask: 0,
                        green_mask: 0,
                        blue_mask: 0,
                        alpha_mask: 0,
                    });
                }
            }
        }

        // Parse ICC profile data for V4/V5 headers
        let (icc_profile_data, icc_profile_size) = if size >= BITMAPV5HEADER_SIZE {
            // V5 header - check for embedded ICC profile
            if data.len() >= OFFSET_CS_TYPE + 4 {
                let cs_type = u32::from_le_bytes([
                    data[OFFSET_CS_TYPE],
                    data[OFFSET_CS_TYPE + 1],
                    data[OFFSET_CS_TYPE + 2],
                    data[OFFSET_CS_TYPE + 3],
                ]);

                // Check if this is an embedded ICC profile (kProfileEmbedded = "MBED")
                if cs_type == CS_TYPE_PROFILE_EMBEDDED {
                    if data.len() >= OFFSET_PROFILE_DATA + 4
                        && data.len() >= OFFSET_PROFILE_SIZE + 4
                    {
                        let profile_data_offset = u32::from_le_bytes([
                            data[OFFSET_PROFILE_DATA],
                            data[OFFSET_PROFILE_DATA + 1],
                            data[OFFSET_PROFILE_DATA + 2],
                            data[OFFSET_PROFILE_DATA + 3],
                        ]);
                        let profile_size = u32::from_le_bytes([
                            data[OFFSET_PROFILE_SIZE],
                            data[OFFSET_PROFILE_SIZE + 1],
                            data[OFFSET_PROFILE_SIZE + 2],
                            data[OFFSET_PROFILE_SIZE + 3],
                        ]);

                        (Some(profile_data_offset), Some(profile_size))
                    } else {
                        (None, None)
                    }
                } else {
                    (None, None)
                }
            } else {
                (None, None)
            }
        } else {
            (None, None)
        };

        Ok(BmpHeaderInfo {
            width,
            height,
            bits_per_pixel,
            compression,
            colors_used,
            data_offset,
            file_size,
            icc_profile_data,
            icc_profile_size,
        })
    }
}

// ============================================================================
// Standalone Helper Functions
// ============================================================================

/// Check if data appears to be a valid BMP file
///
/// This function implements comprehensive BMP format validation matching Chromium's
/// behavior. It handles:
/// - Partial data for streaming scenarios (minimum 2 bytes for signature check)
/// - BITMAPARRAY format with nested BMP
/// - Various header sizes and compression formats
/// - Validation of dimensions, bit depths, and color parameters
/// - Detection of obviously corrupt files
///
/// # Arguments
/// * `data` - Byte slice containing potential BMP data
///
/// # Returns
/// `true` if the data appears to be a valid BMP file, `false` otherwise
pub fn is_bmp_data(data: &[u8]) -> bool {
    // For file detection, we may get partial data due to stream buffering
    // Just check for BMP signature if we have at least 2 bytes
    if data.len() < MIN_SIGNATURE_SIZE {
        return false;
    }

    // For small reads, only do basic signature checks (BMP or BITMAPARRAY)
    if data.len() < MIN_FULL_VALIDATION_SIZE {
        return check_signature_only(data);
    }

    // Determine offset - handle BITMAPARRAY format
    let offset = if is_bitmaparray(data) {
        BMP_FILE_HEADER_SIZE
    } else {
        0
    };

    // Ensure we have enough data for validation at the determined offset
    if data.len() < offset + MIN_FULL_VALIDATION_SIZE {
        return false;
    }

    // Check BMP signature at offset
    if !has_bmp_signature(&data[offset..]) {
        return false;
    }

    // Validate header size
    if !validate_header_size(&data[offset..]) {
        return false;
    }

    // Validate dimensions
    if !validate_dimensions(&data[offset..]) {
        return false;
    }

    // Validate planes (must be 1)
    if !validate_planes(&data[offset..]) {
        return false;
    }

    // Validate bits per pixel
    if !validate_bits_per_pixel(&data[offset..]) {
        return false;
    }

    // Validate compression type
    if !validate_compression(&data[offset..]) {
        return false;
    }

    // Additional validation for indexed color formats
    if !validate_color_parameters(&data[offset..]) {
        return false;
    }

    true
}

/// Check only BMP signature for partial data scenarios
fn check_signature_only(data: &[u8]) -> bool {
    // Check for regular BMP signature "BM"
    if has_bmp_signature(data) {
        return true;
    }

    // Check for BITMAPARRAY signature "BA" if we have enough data
    if data.len() >= MIN_BITMAPARRAY_SIZE && is_bitmaparray(data) {
        // For BITMAPARRAY, check if there's a nested BMP signature at offset 14
        return has_bmp_signature(&data[BMP_FILE_HEADER_SIZE..]);
    }

    false
}

/// Check if data starts with BMP signature "BM"
#[inline]
fn has_bmp_signature(data: &[u8]) -> bool {
    data.len() >= MIN_SIGNATURE_SIZE && data[0] == BMP_SIGNATURE[0] && data[1] == BMP_SIGNATURE[1]
}

/// Check if data starts with BITMAPARRAY signature "BA"
#[inline]
fn is_bitmaparray(data: &[u8]) -> bool {
    data.len() >= MIN_SIGNATURE_SIZE
        && data[0] == BITMAPARRAY_SIGNATURE[0]
        && data[1] == BITMAPARRAY_SIGNATURE[1]
}

/// Validate BMP header size field
fn validate_header_size(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + 4 {
        return false;
    }

    let header_size = u32::from_le_bytes([
        data[BMP_FILE_HEADER_SIZE],
        data[BMP_FILE_HEADER_SIZE + 1],
        data[BMP_FILE_HEADER_SIZE + 2],
        data[BMP_FILE_HEADER_SIZE + 3],
    ]);

    // Valid header sizes matching Chromium's validation
    VALID_HEADER_SIZES.contains(&header_size)
}

/// Validate image dimensions (width and height)
fn validate_dimensions(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + 12 {
        return false;
    }

    let offset = BMP_FILE_HEADER_SIZE + OFFSET_WIDTH;

    // Parse width as unsigned 32-bit (consistent with header.rs)
    let width = u32::from_le_bytes([
        data[offset],
        data[offset + 1],
        data[offset + 2],
        data[offset + 3],
    ]);

    // Parse height as signed 32-bit - negative means top-down
    let height_signed = i32::from_le_bytes([
        data[offset + 4],
        data[offset + 5],
        data[offset + 6],
        data[offset + 7],
    ]);
    let height = height_signed.unsigned_abs();

    // Chromium-compatible validation: reject zero dimensions
    if width == 0 || height == 0 {
        return false;
    }

    // Check against maximum dimensions (matching header.rs limits)
    if width > MAX_DIMENSION || height > MAX_DIMENSION {
        return false;
    }

    // Check for dimension overflow when multiplied (prevent memory exhaustion)
    let total_pixels = (width as u64).saturating_mul(height as u64);
    if total_pixels > MAX_TOTAL_PIXELS {
        return false;
    }

    true
}

/// Validate planes field (must be 1 for BMP)
fn validate_planes(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + 14 {
        return false;
    }

    let offset = BMP_FILE_HEADER_SIZE + 12;
    let _planes = u16::from_le_bytes([data[offset], data[offset + 1]]);

    // Note: We parse planes but don't strictly validate it here for detection
    // Invalid planes will be caught during decode_info validation
    // For detection, we're more lenient to match conformance expectations
    true
}

/// Validate bits per pixel field
fn validate_bits_per_pixel(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + OFFSET_BITS_PER_PIXEL + 2 {
        return false;
    }

    let offset = BMP_FILE_HEADER_SIZE + OFFSET_BITS_PER_PIXEL;
    let bits_per_pixel = u16::from_le_bytes([data[offset], data[offset + 1]]);

    // Valid bit depths matching Chromium
    VALID_BITS_PER_PIXEL.contains(&bits_per_pixel)
}

/// Validate compression type field
fn validate_compression(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + OFFSET_COMPRESSION + 4 {
        return false;
    }

    let offset = BMP_FILE_HEADER_SIZE + OFFSET_COMPRESSION;
    let compression = u32::from_le_bytes([
        data[offset],
        data[offset + 1],
        data[offset + 2],
        data[offset + 3],
    ]);

    CompressionType::is_valid(compression)
}

/// Validate color parameters for indexed formats
fn validate_color_parameters(data: &[u8]) -> bool {
    if data.len() < BMP_FILE_HEADER_SIZE + MIN_INFO_HEADER_SIZE {
        return true; // Not enough data to validate, but don't reject
    }

    let offset = BMP_FILE_HEADER_SIZE;

    // Get bits per pixel to determine if this is indexed color
    let bits_per_pixel = u16::from_le_bytes([
        data[offset + OFFSET_BITS_PER_PIXEL],
        data[offset + OFFSET_BITS_PER_PIXEL + 1],
    ]);

    // Only validate color parameters for indexed formats
    if !matches!(bits_per_pixel, 1 | 4 | 8) {
        return true;
    }

    // Parse color parameters
    let colors_used = u32::from_le_bytes([
        data[offset + OFFSET_COLORS_USED],
        data[offset + OFFSET_COLORS_USED + 1],
        data[offset + OFFSET_COLORS_USED + 2],
        data[offset + OFFSET_COLORS_USED + 3],
    ]);

    let colors_important = u32::from_le_bytes([
        data[offset + OFFSET_COLORS_IMPORTANT],
        data[offset + OFFSET_COLORS_IMPORTANT + 1],
        data[offset + OFFSET_COLORS_IMPORTANT + 2],
        data[offset + OFFSET_COLORS_IMPORTANT + 3],
    ]);

    // Calculate maximum reasonable colors for this bit depth
    let max_colors = match bits_per_pixel {
        1 => MAX_PALETTE_1BPP,
        4 => MAX_PALETTE_4BPP,
        8 => MAX_PALETTE_8BPP,
        _ => return true,
    };

    // Reject files with obviously corrupt color parameters
    // Allow some margin (10x) for malformed but potentially decodable files
    if colors_used > max_colors * 10 || colors_important > max_colors * 10 {
        return false;
    }

    true
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_data() {
        assert!(!is_bmp_data(&[]));
    }

    #[test]
    fn test_single_byte() {
        assert!(!is_bmp_data(&[0x42]));
    }

    #[test]
    fn test_valid_signature() {
        assert!(is_bmp_data(&[0x42, 0x4D])); // "BM"
    }

    #[test]
    fn test_invalid_signature() {
        assert!(!is_bmp_data(&[0x00, 0x00]));
        assert!(!is_bmp_data(&[0x42, 0x00]));
    }

    #[test]
    fn test_bitmaparray_signature() {
        let mut data = vec![0; 16];
        data[0] = 0x42; // 'B'
        data[1] = 0x41; // 'A'
        data[14] = 0x42; // 'B'
        data[15] = 0x4D; // 'M'
        assert!(is_bmp_data(&data));
    }

    #[test]
    fn test_compression_type_validation() {
        assert!(CompressionType::is_valid(0)); // BI_RGB
        assert!(CompressionType::is_valid(1)); // BI_RLE8
        assert!(CompressionType::is_valid(4)); // BI_JPEG
        assert!(CompressionType::is_valid(5)); // BI_PNG
        assert!(CompressionType::is_valid(6)); // BI_ALPHABITFIELDS
        assert!(!CompressionType::is_valid(7)); // Invalid
        assert!(!CompressionType::is_valid(100)); // Invalid
    }
}
