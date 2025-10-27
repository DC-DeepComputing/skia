// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! PNG decoder integration using the png crate
//!
//! This module provides PNG decoding functionality for BMP files containing
//! embedded PNG data. Uses the png crate for pure-Rust PNG decoding.

use super::bmp_constants::{ALPHA_OPAQUE, BYTES_PER_PIXEL_RGB, BYTES_PER_PIXEL_RGBA};
use super::bmp_types::BmpError;
use png::{ColorType, Decoder, Transformations};
use std::io::Cursor;

/// Decode PNG data embedded in a BMP file
///
/// # Arguments
/// * `png_data` - Raw PNG-encoded bytes (must start with PNG signature)
/// * `expected_width` - Expected image width from BMP header (for validation)
/// * `expected_height` - Expected image height from BMP header (for validation)
///
/// # Returns
/// Decoded RGBA pixels in row-major order (4 bytes per pixel: R, G, B, A)
///
/// # Errors
/// Returns `BmpError::PngDecodingFailed` if:
/// - PNG data is invalid or corrupted
/// - Decoding fails for any reason
/// - Dimensions don't match expectations
pub fn decode_png(
    png_data: &[u8],
    expected_width: u32,
    expected_height: u32,
) -> Result<Vec<u8>, BmpError> {
    let cursor = Cursor::new(png_data);

    let mut decoder = Decoder::new(cursor);
    decoder.set_transformations(Transformations::EXPAND | Transformations::ALPHA);

    let mut reader = decoder
        .read_info()
        .map_err(|e| BmpError::PngDecodingFailed(format!("PNG decode error: {}", e)))?;

    let info = reader.info();
    let width = info.width;
    let height = info.height;

    if width != expected_width || height != expected_height {
        return Err(BmpError::PngDecodingFailed(format!(
            "PNG dimensions ({}x{}) don't match BMP header ({}x{})",
            width, height, expected_width, expected_height
        )));
    }

    let mut buffer = vec![0u8; reader.output_buffer_size()];

    let info_after = reader
        .next_frame(&mut buffer)
        .map_err(|e| BmpError::PngDecodingFailed(format!("PNG frame decode error: {}", e)))?;

    let bytes_read = info_after.buffer_size();
    buffer.truncate(bytes_read);

    let rgba_pixels = match info_after.color_type {
        ColorType::Rgba => buffer,
        ColorType::Rgb => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for chunk in buffer.chunks_exact(BYTES_PER_PIXEL_RGB) {
                rgba.push(chunk[0]);
                rgba.push(chunk[1]);
                rgba.push(chunk[2]);
                rgba.push(ALPHA_OPAQUE);
            }
            rgba
        }
        ColorType::Grayscale => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for &gray in &buffer {
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(ALPHA_OPAQUE);
            }
            rgba
        }
        ColorType::GrayscaleAlpha => {
            let mut rgba = Vec::with_capacity((width * height) as usize * BYTES_PER_PIXEL_RGBA);
            for chunk in buffer.chunks_exact(2) {
                let gray = chunk[0];
                let alpha = chunk[1];
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(gray);
                rgba.push(alpha);
            }
            rgba
        }
        ColorType::Indexed => {
            return Err(BmpError::PngDecodingFailed(
                "Indexed PNG not properly expanded".to_string(),
            ));
        }
    };

    Ok(rgba_pixels)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_invalid_png_data() {
        let invalid_data = b"not a png";
        let result = decode_png(invalid_data, 100, 100);
        assert!(result.is_err());
    }

    #[test]
    fn test_empty_png_data() {
        let result = decode_png(&[], 100, 100);
        assert!(result.is_err());
    }
}
