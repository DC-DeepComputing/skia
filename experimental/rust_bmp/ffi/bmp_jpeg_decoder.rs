// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! JPEG decoder integration using zune-jpeg
//!
//! This module provides JPEG decoding functionality for BMP files containing
//! embedded JPEG data. Uses zune-jpeg for pure-Rust JPEG decoding.

use super::bmp_constants::{ALPHA_OPAQUE, BYTES_PER_PIXEL_RGB, BYTES_PER_PIXEL_RGBA};
use super::bmp_types::BmpError;
use zune_jpeg::zune_core::bytestream::ZCursor;
use zune_jpeg::zune_core::colorspace::ColorSpace;
use zune_jpeg::zune_core::options::DecoderOptions;
use zune_jpeg::JpegDecoder;

/// Decode JPEG data embedded in a BMP file
///
/// # Arguments
/// * `jpeg_data` - Raw JPEG-encoded bytes (must start with JPEG SOI marker 0xFF 0xD8)
/// * `expected_width` - Expected image width from BMP header (for validation)
/// * `expected_height` - Expected image height from BMP header (for validation)
///
/// # Returns
/// Decoded RGBA pixels in row-major order (4 bytes per pixel: R, G, B, A)
///
/// # Errors
/// Returns `BmpError::JpegDecodingFailed` if:
/// - JPEG data is invalid or corrupted
/// - Decoding fails for any reason
/// - Dimensions don't match expectations
pub fn decode_jpeg(
    jpeg_data: &[u8],
    expected_width: u32,
    expected_height: u32,
) -> Result<Vec<u8>, BmpError> {
    let options = DecoderOptions::new_fast();
    let cursor = ZCursor::new(jpeg_data);
    let mut decoder = JpegDecoder::new_with_options(cursor, options);

    let pixels = decoder
        .decode()
        .map_err(|e| BmpError::JpegDecodingFailed(format!("JPEG decode error: {}", e)))?;

    let dimensions = decoder
        .dimensions()
        .ok_or_else(|| BmpError::JpegDecodingFailed("Failed to get JPEG dimensions".to_string()))?;

    let width = dimensions.0 as u32;
    let height = dimensions.1 as u32;

    if width != expected_width || height != expected_height {
        return Err(BmpError::JpegDecodingFailed(format!(
            "JPEG dimensions ({}x{}) don't match BMP header ({}x{})",
            width, height, expected_width, expected_height
        )));
    }

    let colorspace = decoder.output_colorspace();

    let rgba_pixels = match colorspace {
        Some(ColorSpace::RGB) => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for chunk in pixels.chunks_exact(BYTES_PER_PIXEL_RGB) {
                rgba.push(chunk[0]);
                rgba.push(chunk[1]);
                rgba.push(chunk[2]);
                rgba.push(ALPHA_OPAQUE);
            }
            rgba
        }
        Some(ColorSpace::Luma) => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for &gray in &pixels {
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(ALPHA_OPAQUE);
            }
            rgba
        }
        Some(ColorSpace::YCbCr) => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for chunk in pixels.chunks_exact(BYTES_PER_PIXEL_RGB) {
                rgba.push(chunk[0]);
                rgba.push(chunk[1]);
                rgba.push(chunk[2]);
                rgba.push(ALPHA_OPAQUE);
            }
            rgba
        }
        Some(ColorSpace::RGBA) => pixels,
        Some(ColorSpace::CMYK) | Some(ColorSpace::YCCK) => {
            return Err(BmpError::JpegDecodingFailed(
                "CMYK JPEG images are not supported".to_string(),
            ));
        }
        Some(other) => {
            return Err(BmpError::JpegDecodingFailed(format!(
                "Unsupported JPEG colorspace: {:?}",
                other
            )));
        }
        None => {
            return Err(BmpError::JpegDecodingFailed(
                "Failed to get JPEG colorspace".to_string(),
            ));
        }
    };

    Ok(rgba_pixels)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_invalid_jpeg_data() {
        let invalid_data = b"not a jpeg";
        let result = decode_jpeg(invalid_data, 100, 100);
        assert!(result.is_err());
    }

    #[test]
    fn test_empty_jpeg_data() {
        let result = decode_jpeg(&[], 100, 100);
        assert!(result.is_err());
    }
}
