// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! BMP decoder implementation with behavioral parity to Chromium's BMPImageDecoder.

use super::bmp_constants::{
    ALPHA_DEFAULT, BYTES_PER_PIXEL_RGB, BYTES_PER_PIXEL_RGBA, JPEG_MIN_SIGNATURE_SIZE,
    JPEG_SOI_MARKER_1, JPEG_SOI_MARKER_2, PALETTE_ENTRY_SIZE, PNG_MIN_SIGNATURE_SIZE,
    PNG_SIGNATURE, ROW_ALIGNMENT_BITS, ROW_ALIGNMENT_BYTES,
};
use super::bmp_header::HeaderParser;
use super::bmp_icc;
use super::bmp_jpeg_decoder;
use super::bmp_png_decoder;
use super::bmp_types::{BmpError, BmpHeaderInfo, CompressionType};
use super::{AlphaType, ColorType};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EmbeddedFormat {
    Jpeg,
    Png,
}

#[derive(Debug, Clone)]
pub struct DecodedImageInternal {
    pub pixels: Vec<u8>,
    pub bytes_per_pixel: u32,
    pub data_is_incomplete: bool,
    pub embedded_format: Option<EmbeddedFormat>,
}

#[derive(Debug, Clone)]
pub struct ImageInfoInternal {
    pub width: u32,
    pub height: u32,
    pub color_type: ColorType,
    pub alpha_type: AlphaType,
}

#[derive(Debug, Clone, Copy)]
pub struct RgbTriple {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

/// Decompression handler for BMP data
pub struct DecompressionHandler {
    compression_type: CompressionType,
    red_mask: Option<u32>,
    green_mask: Option<u32>,
    blue_mask: Option<u32>,
    alpha_mask: Option<u32>,
    color_table: Vec<RgbTriple>,
}

pub struct BmpDecoderImpl {
    is_size_available: bool,
    data: Option<Vec<u8>>,
    pub width: u32,
    pub height: u32,
    header_parser: Option<HeaderParser>,
}

impl BmpDecoderImpl {
    pub fn new() -> Self {
        Self {
            is_size_available: false,
            data: None,
            width: 0,
            height: 0,
            header_parser: None,
        }
    }

    pub fn decode_info(&mut self, data: &[u8]) -> Result<ImageInfoInternal, BmpError> {
        self.data = Some(data.to_vec());

        let mut parser = HeaderParser::new();
        let header_info = parser.parse_header(data)?;

        self.width = header_info.width;
        self.height = header_info.height;
        self.is_size_available = true;
        self.header_parser = Some(parser);

        let (color_type, alpha_type) = self.determine_color_format(&header_info)?;

        Ok(ImageInfoInternal {
            width: header_info.width,
            height: header_info.height,
            color_type,
            alpha_type,
        })
    }

    /// Decode pixel data
    pub fn decode_pixels(&mut self, data: &[u8]) -> Result<DecodedImageInternal, BmpError> {
        if !self.is_size_available {
            self.decode_info(data)?;
        }

        let header_parser = self.header_parser.as_ref().ok_or(BmpError::InvalidHeader)?;
        let header_info = header_parser.get_header_info()?;

        let mut decompression_handler = if header_info.compression == CompressionType::BitFields {
            let color_masks = header_parser
                .get_color_masks()
                .ok_or(BmpError::InvalidHeader)?;
            DecompressionHandler::new_with_masks(
                header_info.compression,
                Some(color_masks.red_mask),
                Some(color_masks.green_mask),
                Some(color_masks.blue_mask),
                Some(color_masks.alpha_mask),
            )
        } else {
            DecompressionHandler::new(header_info.compression)
        };

        if header_info.bits_per_pixel <= 8 {
            let color_table = self.parse_color_palette(data, &header_info)?;
            decompression_handler.set_color_table(color_table);
        }

        // Check that we have enough data for pixel data
        let pixel_data_offset = header_info.data_offset as usize;
        if pixel_data_offset >= data.len() {
            return Err(BmpError::InvalidDataOffset);
        }

        let bytes_per_row = ((header_info.bits_per_pixel as u32 * self.width
            + (ROW_ALIGNMENT_BITS - 1))
            / ROW_ALIGNMENT_BITS)
            * ROW_ALIGNMENT_BYTES as u32;
        let _expected_pixel_size = bytes_per_row * self.height;

        // Very lenient pixel data validation
        // Chromium is extremely lenient with IE6 compatibility files
        let available_pixel_data = data.len() - pixel_data_offset;
        if available_pixel_data == 0 {
            return Err(BmpError::InsufficientData);
        }

        // For uncompressed formats, be extremely lenient like Chromium
        // Only require minimal data - this handles IE6 compatibility files and 1x1 images
        if header_info.compression == CompressionType::None {
            // Calculate minimum required for this specific image
            let min_required = ((bytes_per_row as usize) / 4).clamp(1, 4);

            if available_pixel_data < min_required {
                return Err(BmpError::InsufficientData);
            }
        }

        // Detect and decode embedded JPEG/PNG using pure Rust implementations
        // TODO: Delegate to corresponding SkCodec.
        let pixel_data = &data[pixel_data_offset..];
        if let Some(embedded_format) = self.contains_embedded_format(pixel_data)? {
            match embedded_format {
                EmbeddedFormat::Jpeg => {
                    let decoded_pixels =
                        bmp_jpeg_decoder::decode_jpeg(pixel_data, self.width, self.height)?;

                    return Ok(DecodedImageInternal {
                        pixels: decoded_pixels,
                        bytes_per_pixel: BYTES_PER_PIXEL_RGBA as u32,
                        data_is_incomplete: false,
                        embedded_format: Some(embedded_format),
                    });
                }
                EmbeddedFormat::Png => {
                    let decoded_pixels =
                        bmp_png_decoder::decode_png(pixel_data, self.width, self.height)?;

                    return Ok(DecodedImageInternal {
                        pixels: decoded_pixels,
                        bytes_per_pixel: BYTES_PER_PIXEL_RGBA as u32,
                        data_is_incomplete: false,
                        embedded_format: Some(embedded_format),
                    });
                }
            }
        }

        let pixel_data = &data[pixel_data_offset..];

        // Determine if this is a top-down BMP (negative height in original header)
        let is_top_down = self.is_top_down_bmp(data)?;

        // Decode pixels based on compression type
        let mut decoded_pixels = decompression_handler.decompress_pixels(
            pixel_data,
            self.width,
            self.height,
            header_info.bits_per_pixel,
            is_top_down,
        )?;

        // Apply ICC color profile transformation if present (V4/V5 headers)
        // TODO: Evaluate if delegation of ICC profile handling is suitable
        // to help reuse in SkPngRustCodec which also requires ICC profile support.
        if let (Some(icc_offset), Some(icc_size)) =
            (header_info.icc_profile_data, header_info.icc_profile_size)
        {
            if let Some(icc_profile) = bmp_icc::extract_icc_profile(data, icc_offset, icc_size) {
                if let Some(transformed) = bmp_icc::apply_icc_transform(
                    &icc_profile,
                    &decoded_pixels,
                    self.width,
                    self.height,
                ) {
                    decoded_pixels = transformed;
                }
            }
        }

        // Chromium compatibility: file_size of 0 means size wasn't set by encoder
        let data_is_incomplete = if header_info.file_size > 0 {
            data.len() < header_info.file_size as usize
        } else {
            false
        };

        Ok(DecodedImageInternal {
            pixels: decoded_pixels,
            bytes_per_pixel: BYTES_PER_PIXEL_RGB as u32,
            data_is_incomplete,
            embedded_format: None,
        })
    }

    /// Parse color palette from BMP data
    fn parse_color_palette(
        &self,
        data: &[u8],
        header_info: &BmpHeaderInfo,
    ) -> Result<Vec<RgbTriple>, BmpError> {
        let header_size = if data.len() >= 18 {
            u32::from_le_bytes([data[14], data[15], data[16], data[17]]) as usize
        } else {
            40
        };

        let palette_offset = 14 + header_size;

        // For palette-based images, check if palette data exists
        if palette_offset >= data.len() {
            return Err(BmpError::InsufficientData);
        }

        let max_colors = 1u32 << header_info.bits_per_pixel;
        let colors_used = if header_info.colors_used == 0 {
            max_colors
        } else {
            header_info.colors_used.min(max_colors)
        };

        let palette_size = colors_used as usize * PALETTE_ENTRY_SIZE;

        // Check if we have enough data for the required palette
        if palette_offset + palette_size > data.len() {
            return Err(BmpError::InsufficientData);
        }

        self.read_palette_entries(data, palette_offset, colors_used as usize)
    }

    /// Read palette entries from BMP data
    fn read_palette_entries(
        &self,
        data: &[u8],
        offset: usize,
        num_colors: usize,
    ) -> Result<Vec<RgbTriple>, BmpError> {
        let mut color_table = Vec::with_capacity(num_colors);

        for i in 0..num_colors {
            let color_offset = offset + i * PALETTE_ENTRY_SIZE;

            if color_offset + 3 < data.len() {
                let b = data[color_offset];
                let g = data[color_offset + 1];
                let r = data[color_offset + 2];

                color_table.push(RgbTriple { r, g, b });
            } else {
                color_table.push(RgbTriple { r: 0, g: 0, b: 0 });
            }
        }

        Ok(color_table)
    }

    /// Detect embedded JPEG/PNG data and return the format type
    fn contains_embedded_format(
        &self,
        pixel_data: &[u8],
    ) -> Result<Option<EmbeddedFormat>, BmpError> {
        if pixel_data.len() >= JPEG_MIN_SIGNATURE_SIZE
            && pixel_data[0] == JPEG_SOI_MARKER_1
            && pixel_data[1] == JPEG_SOI_MARKER_2
        {
            return Ok(Some(EmbeddedFormat::Jpeg));
        }

        if pixel_data.len() >= PNG_MIN_SIGNATURE_SIZE
            && pixel_data[0..PNG_MIN_SIGNATURE_SIZE] == PNG_SIGNATURE
        {
            return Ok(Some(EmbeddedFormat::Png));
        }

        // Check for JFIF/Exif markers in JPEG files
        if pixel_data.len() >= 8 {
            for i in 0..pixel_data.len().saturating_sub(4) {
                if &pixel_data[i..i + 4] == b"JFIF" || &pixel_data[i..i + 4] == b"Exif" {
                    return Ok(Some(EmbeddedFormat::Jpeg));
                }
            }
        }

        Ok(None)
    }

    /// Check if this is a top-down BMP (negative height in original header)
    fn is_top_down_bmp(&self, data: &[u8]) -> Result<bool, BmpError> {
        if data.len() < 26 {
            return Err(BmpError::InsufficientData);
        }

        let height_bytes = [data[22], data[23], data[24], data[25]];
        let original_height = i32::from_le_bytes(height_bytes);

        Ok(original_height < 0)
    }

    /// FFI-compatible decode_info method - returns tuple for C++ interop
    pub fn decode_info_ffi(
        &mut self,
        data: &[u8],
    ) -> Result<(u32, u32, ColorType, AlphaType), BmpError> {
        let info = self.decode_info(data)?;
        Ok((info.width, info.height, info.color_type, info.alpha_type))
    }

    /// Determine color format based on BMP characteristics
    fn determine_color_format(
        &self,
        header_info: &BmpHeaderInfo,
    ) -> Result<(ColorType, AlphaType), BmpError> {
        // For embedded JPEG/PNG, always return RGBA (they will be decoded to RGBA)
        if header_info.compression == CompressionType::Jpeg
            || header_info.compression == CompressionType::Png
        {
            return Ok((ColorType::Rgba, AlphaType::Opaque));
        }

        match header_info.bits_per_pixel {
            24 => Ok((ColorType::Bgr, AlphaType::Opaque)),
            32 => Ok((ColorType::Rgba, AlphaType::Unpremul)),
            1 | 4 | 8 => Ok((ColorType::Rgb, AlphaType::Opaque)),
            16 => Ok((ColorType::Rgb, AlphaType::Opaque)),
            _ => Err(BmpError::UnsupportedFeature(format!(
                "{} bits per pixel",
                header_info.bits_per_pixel
            ))),
        }
    }
}

// DecompressionHandler Implementation

impl DecompressionHandler {
    pub fn new(compression_type: CompressionType) -> Self {
        Self {
            compression_type,
            red_mask: None,
            green_mask: None,
            blue_mask: None,
            alpha_mask: None,
            color_table: Vec::new(),
        }
    }

    pub fn new_with_masks(
        compression_type: CompressionType,
        red_mask: Option<u32>,
        green_mask: Option<u32>,
        blue_mask: Option<u32>,
        alpha_mask: Option<u32>,
    ) -> Self {
        Self {
            compression_type,
            red_mask,
            green_mask,
            blue_mask,
            alpha_mask,
            color_table: Vec::new(),
        }
    }

    /// Set the color palette for palette-based images
    pub fn set_color_table(&mut self, color_table: Vec<RgbTriple>) {
        self.color_table = color_table;
    }

    /// Decompress pixel data based on compression type
    pub fn decompress_pixels(
        &mut self,
        data: &[u8],
        width: u32,
        height: u32,
        bits_per_pixel: u16,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        let abs_height = height; // height parameter should already be absolute

        let result = match self.compression_type {
            CompressionType::None => {
                self.decompress_uncompressed(data, width, abs_height, bits_per_pixel, is_top_down)
            }
            CompressionType::Rle8 => self.decompress_rle8(data, width, abs_height, is_top_down),
            CompressionType::Rle4 => self.decompress_rle4(data, width, abs_height, is_top_down),
            CompressionType::Jpeg => Err(BmpError::UnsupportedFeature(
                "JPEG compression requires external processor".to_string(),
            )),
            CompressionType::Png => Err(BmpError::UnsupportedFeature(
                "PNG compression requires external processor".to_string(),
            )),
            CompressionType::BitFields => {
                // For BITFIELDS, Chromium has special handling based on bit depth
                match bits_per_pixel {
                    1 => {
                        // For 1bpp, Chromium always treats BI_BITFIELDS as standard uncompressed
                        // regardless of mask values, because 1bpp is inherently palette-based
                        self.decompress_uncompressed(
                            data,
                            width,
                            abs_height,
                            bits_per_pixel,
                            is_top_down,
                        )
                    }
                    4 | 8 => {
                        // For other palette bit depths, also treat as uncompressed
                        self.decompress_uncompressed(
                            data,
                            width,
                            abs_height,
                            bits_per_pixel,
                            is_top_down,
                        )
                    }
                    16 | 24 | 32 => {
                        // For direct color, check if masks are meaningful
                        if self.all_masks_zero() {
                            // Zero masks means treat as uncompressed with default layout
                            self.decompress_uncompressed(
                                data,
                                width,
                                abs_height,
                                bits_per_pixel,
                                is_top_down,
                            )
                        } else {
                            // Non-zero masks require proper BITFIELDS processing
                            self.decompress_bitfields(data, width, abs_height, bits_per_pixel)
                        }
                    }
                    _ => Err(BmpError::UnsupportedFeature(format!(
                        "BITFIELDS with {} bits per pixel",
                        bits_per_pixel
                    ))),
                }
            }
        };

        result
    }

    fn decompress_uncompressed(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        bits_per_pixel: u16,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        match bits_per_pixel {
            1 => self.decompress_1bit_palette(data, width, height, is_top_down),
            4 => self.decompress_4bit_palette(data, width, height, is_top_down),
            8 => self.decompress_8bit_palette(data, width, height, is_top_down),
            16 => self.decompress_16bit(data, width, height, is_top_down),
            24 => self.decompress_24bit(data, width, height, is_top_down),
            32 => self.decompress_32bit(data, width, height, is_top_down),
            _ => Err(BmpError::UnsupportedFeature(format!(
                "{} bits per pixel",
                bits_per_pixel
            ))),
        }
    }

    fn decompress_1bit_palette(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // For 1-bit images, each byte contains 8 pixels
        // We need to convert palette indices to RGB values

        // Be more lenient with data size requirements like Chromium
        let min_required = std::cmp::max(1, (width * height).div_ceil(8));
        if (data.len() as u32) < min_required {
            return Err(BmpError::InsufficientData);
        }

        // Ensure we have a color table with at least 2 colors for 1-bit
        if self.color_table.is_empty() {
            // If no color table, use default B&W palette like Chromium
            let default_palette = vec![
                RgbTriple { r: 0, g: 0, b: 0 }, // Black for 0
                RgbTriple {
                    r: 255,
                    g: 255,
                    b: 255,
                }, // White for 1
            ];
            return self.decompress_1bit_with_palette(
                data,
                width,
                height,
                is_top_down,
                &default_palette,
            );
        }

        self.decompress_1bit_with_palette(data, width, height, is_top_down, &self.color_table)
    }

    fn decompress_1bit_with_palette(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
        palette: &[RgbTriple],
    ) -> Result<Vec<u8>, BmpError> {
        // Pre-allocate with exact size and use direct indexing
        let total_pixels = (width * height) as usize;
        let mut result = vec![0u8; total_pixels * 3];
        let mut output_idx = 0;

        // Calculate row size with padding
        let bits_per_row = width;
        let bytes_per_row = bits_per_row.div_ceil(8);
        let row_size = bytes_per_row.div_ceil(4) * 4;

        for y in 0..height {
            let row_offset = (y * row_size) as usize;

            for x in 0..width {
                let byte_index = row_offset + (x / 8) as usize;
                let bit_position = 7 - (x % 8);

                if byte_index < data.len() {
                    let byte_value = data[byte_index];
                    let palette_index = ((byte_value >> bit_position) & 1) as usize;

                    // Convert palette index to RGB
                    if palette_index < palette.len() {
                        let color = palette[palette_index];
                        result[output_idx] = color.r;
                        result[output_idx + 1] = color.g;
                        result[output_idx + 2] = color.b;
                    } else {
                        result[output_idx] = 0;
                        result[output_idx + 1] = 0;
                        result[output_idx + 2] = 0;
                    }
                } else {
                    // If we run out of data, fill with first palette color
                    let color = if !palette.is_empty() {
                        palette[0]
                    } else {
                        RgbTriple { r: 0, g: 0, b: 0 }
                    };
                    result[output_idx] = color.r;
                    result[output_idx + 1] = color.g;
                    result[output_idx + 2] = color.b;
                }
                output_idx += 3;
            }
        }

        Ok(result)
    }

    fn decompress_4bit_palette(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // For 4-bit images, each byte contains 2 pixels
        // We need to convert palette indices to RGB values

        let bits_per_row = width * 4; // 4 bits per pixel
        let bytes_per_row = bits_per_row.div_ceil(8); // Round up to bytes
        let row_size = bytes_per_row.div_ceil(4) * 4; // Pad to 4-byte boundary

        // Be more lenient with data size like Chromium
        let min_required = std::cmp::max(1, (width * height * 4).div_ceil(8));
        if (data.len() as u32) < min_required {
            return Err(BmpError::InsufficientData);
        }

        // Ensure we have a color table - if missing, create grayscale palette like Chromium
        let palette = if self.color_table.is_empty() {
            // Create default 16-color grayscale palette
            let mut default_palette = Vec::with_capacity(16);
            for i in 0..16 {
                let gray_value = (i * 17) as u8; // 0, 17, 34, 51, ..., 255
                default_palette.push(RgbTriple {
                    r: gray_value,
                    g: gray_value,
                    b: gray_value,
                });
            }
            default_palette
        } else {
            self.color_table.clone()
        };

        // Ensure palette has at least 16 colors for 4-bit
        if palette.len() < 16 {
            return Err(BmpError::UnsupportedFeature(
                "Insufficient color table for 4-bit BMP".to_string(),
            ));
        }

        // Pre-allocate with exact size and use direct indexing
        let total_pixels = (width * height) as usize;
        let mut result = vec![0u8; total_pixels * 3];
        let mut output_idx = 0;

        for y in 0..height {
            let row_offset = (y * row_size) as usize;

            for x in 0..width {
                let byte_index = row_offset + (x / 2) as usize;
                let is_high_nibble = (x % 2) == 0;

                if byte_index < data.len() {
                    let byte_value = data[byte_index];
                    let palette_index = if is_high_nibble {
                        ((byte_value >> 4) & 0x0F) as usize
                    } else {
                        (byte_value & 0x0F) as usize
                    };

                    // Convert palette index to RGB
                    if palette_index < palette.len() {
                        let color = palette[palette_index];
                        result[output_idx] = color.r;
                        result[output_idx + 1] = color.g;
                        result[output_idx + 2] = color.b;
                    } else {
                        result[output_idx] = 0;
                        result[output_idx + 1] = 0;
                        result[output_idx + 2] = 0;
                    }
                } else {
                    // If we run out of data, fill with first palette color
                    let color = palette[0];
                    result[output_idx] = color.r;
                    result[output_idx + 1] = color.g;
                    result[output_idx + 2] = color.b;
                }
                output_idx += 3;
            }
        }

        Ok(result)
    }

    fn decompress_8bit_palette(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // For 8-bit images, each byte is one pixel
        // We need to convert palette indices to RGB values

        let bytes_per_row = width; // 1 byte per pixel
        let row_size = bytes_per_row.div_ceil(4) * 4; // Pad to 4-byte boundary
        let expected_size = row_size * height;

        if data.len() < expected_size as usize {
            return Err(BmpError::InsufficientData);
        }

        // Ensure we have a color table
        if self.color_table.is_empty() {
            return Err(BmpError::UnsupportedFeature(
                "Missing color table for 8-bit BMP".to_string(),
            ));
        }

        // Pre-allocate with exact size and use direct indexing
        let total_pixels = (width * height) as usize;
        let mut result = vec![0u8; total_pixels * 3];
        let mut output_idx = 0;

        for y in 0..height {
            let row_offset = (y * row_size) as usize;

            for x in 0..width {
                let pixel_index = row_offset + x as usize;
                if pixel_index >= data.len() {
                    return Err(BmpError::InsufficientData);
                }

                let palette_index = data[pixel_index] as usize;

                // Convert palette index to RGB
                if palette_index < self.color_table.len() {
                    let color = self.color_table[palette_index];
                    result[output_idx] = color.r;
                    result[output_idx + 1] = color.g;
                    result[output_idx + 2] = color.b;
                } else {
                    // Invalid palette index - use black as fallback
                    result[output_idx] = 0;
                    result[output_idx + 1] = 0;
                    result[output_idx + 2] = 0;
                }
                output_idx += 3;
            }
        }

        Ok(result)
    }

    fn decompress_16bit(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // 16-bit BMP can be either RGB565 (5-6-5) or RGB555 (5-5-5)
        // Most common format is RGB565: [RRRRR GGGGGG BBBBB]
        // We'll assume RGB565 unless we detect RGB555 via bit masks

        let bytes_per_pixel = 2; // 16 bits = 2 bytes

        let row_size = (width * bytes_per_pixel).div_ceil(4) * 4;
        let expected_size = row_size * height;

        if data.len() < expected_size as usize {
            return Err(BmpError::InsufficientData);
        }

        //  Pre-allocate with exact size and use direct indexing
        let total_pixels = (width * height) as usize;
        let mut result = vec![0u8; total_pixels * 3];
        let mut output_idx = 0;

        for y in 0..height {
            let row_offset = (y * row_size) as usize;

            for x in 0..width {
                let pixel_offset = row_offset + (x * bytes_per_pixel) as usize;

                if pixel_offset + 1 >= data.len() {
                    return Err(BmpError::InsufficientData);
                }

                let pixel16 = u16::from_le_bytes([data[pixel_offset], data[pixel_offset + 1]]);

                // Decode as RGB565 (most common 16-bit format)
                // Format: [RRRRR GGGGGG BBBBB]
                let r5 = ((pixel16 >> 11) & 0x1F) as u8; // 5 bits for red
                let g6 = ((pixel16 >> 5) & 0x3F) as u8; // 6 bits for green
                let b5 = (pixel16 & 0x1F) as u8; // 5 bits for blue

                // Expand to 8 bits per channel
                let r8 = (r5 << 3) | (r5 >> 2);
                let g8 = (g6 << 2) | (g6 >> 4);
                let b8 = (b5 << 3) | (b5 >> 2);

                // Direct indexing instead of push
                result[output_idx] = r8;
                result[output_idx + 1] = g8;
                result[output_idx + 2] = b8;
                output_idx += 3;
            }
        }

        Ok(result)
    }

    fn decompress_24bit(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        let bytes_per_pixel = 3;

        let row_size = (width * bytes_per_pixel).div_ceil(4) * 4;

        // When width*3 is already 4-byte aligned, we can do a direct copy
        // This is true for widths where (width*3) % 4 == 0
        // Examples: width=4,8,12,16,20,24,28,32,36,40,44,48...
        if row_size == width * bytes_per_pixel {
            let expected_size = (width * height * 3) as usize;
            if data.len() >= expected_size {
                // Direct vector conversion - single memcpy operation
                return Ok(data[..expected_size].to_vec());
            }
        }

        // Slow path: Handle padding for non-aligned widths
        let expected_size = row_size * height;

        if data.len() < expected_size as usize {
            return Err(BmpError::InsufficientData);
        }

        // Pre-allocate exact size to avoid reallocation
        let total_pixels = (width * height * 3) as usize;
        let mut result = Vec::with_capacity(total_pixels);

        for y in 0..height {
            // Read rows in their original order from the BMP file
            let row_offset = (y * row_size) as usize;
            let row_end = row_offset + (width * 3) as usize;

            // Batch copy entire row instead of per-pixel loop
            result.extend_from_slice(&data[row_offset..row_end]);
        }

        Ok(result)
    }

    fn decompress_32bit(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        _is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        let bytes_per_pixel = 4;

        let row_size = (width * bytes_per_pixel).div_ceil(4) * 4;
        let expected_size = row_size * height;

        if data.len() < expected_size as usize {
            return Err(BmpError::InsufficientData);
        }

        // Pre-allocate with exact size and use direct indexing
        let total_pixels = (width * height) as usize;
        let mut result = vec![0u8; total_pixels * 3];
        let mut output_idx = 0;

        for y in 0..height {
            let row_offset = (y * row_size) as usize;

            for x in 0..width {
                let pixel_offset = row_offset + (x * bytes_per_pixel) as usize;

                if pixel_offset + bytes_per_pixel as usize > data.len() {
                    return Err(BmpError::InsufficientData);
                }

                // BMP stores pixels as BGRA, convert to RGB (skip alpha)
                let b = data[pixel_offset];
                let g = data[pixel_offset + 1];
                let r = data[pixel_offset + 2];

                // Direct indexing instead of push
                result[output_idx] = r;
                result[output_idx + 1] = g;
                result[output_idx + 2] = b;
                output_idx += 3;
            }
        }

        Ok(result)
    }

    fn decompress_rle8(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // Decode RLE8 data
        let mut pixels = vec![0u8; (width * height) as usize];
        let mut i = 0;
        let mut pixel_index = 0;

        while i < data.len() && pixel_index < pixels.len() {
            if i + 1 >= data.len() {
                break;
            }

            let count = data[i];
            let value = data[i + 1];

            if count == 0 {
                // Escape code
                match value {
                    0 => {
                        // End of line - move to next row start
                        let current_row = pixel_index / width as usize;
                        pixel_index = (current_row + 1) * width as usize;
                        i += 2;
                    }
                    1 => {
                        // End of bitmap
                        break;
                    }
                    2 => {
                        // Delta - skip pixels
                        if i + 3 < data.len() {
                            let dx = data[i + 2] as usize;
                            let dy = data[i + 3] as usize;
                            pixel_index += dx + dy * width as usize;
                            i += 4;
                        } else {
                            break;
                        }
                    }
                    _ => {
                        // Absolute mode - copy 'value' pixels literally
                        let copy_count = value as usize;
                        let bytes_to_read = copy_count;
                        // Pad to word boundary
                        let padded_bytes = if bytes_to_read % 2 != 0 {
                            bytes_to_read + 1
                        } else {
                            bytes_to_read
                        };

                        if i + 2 + padded_bytes <= data.len() {
                            for j in 0..copy_count {
                                if pixel_index < pixels.len() {
                                    pixels[pixel_index] = data[i + 2 + j];
                                    pixel_index += 1;
                                }
                            }
                            i += 2 + padded_bytes;
                        } else {
                            return Err(BmpError::RleDecodingFailed);
                        }
                    }
                }
            } else {
                // Encoded mode - repeat 'value' 'count' times
                for _ in 0..count {
                    if pixel_index < pixels.len() {
                        pixels[pixel_index] = value;
                        pixel_index += 1;
                    }
                }
                i += 2;
            }
        }

        // Check if we decoded the expected number of pixels
        let expected_pixels = (width * height) as usize;
        if pixel_index < expected_pixels {
            return Err(BmpError::InsufficientData);
        }

        // Convert indexed pixels to RGB using palette
        self.convert_indexed_to_rgb(&pixels, width, height, is_top_down)
    }

    fn decompress_rle4(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // Decode RLE4 data
        let mut pixels = vec![0u8; (width * height) as usize];
        let mut i = 0;
        let mut pixel_index = 0;

        while i < data.len() && pixel_index < pixels.len() {
            if i + 1 >= data.len() {
                break;
            }

            let count = data[i];
            let value = data[i + 1];

            if count == 0 {
                // Escape code
                match value {
                    0 => {
                        // End of line - move to next row start
                        let current_row = pixel_index / width as usize;
                        pixel_index = (current_row + 1) * width as usize;
                        i += 2;
                    }
                    1 => {
                        // End of bitmap
                        break;
                    }
                    2 => {
                        // Delta - skip pixels
                        if i + 3 < data.len() {
                            let dx = data[i + 2] as usize;
                            let dy = data[i + 3] as usize;
                            pixel_index += dx + dy * width as usize;
                            i += 4;
                        } else {
                            break;
                        }
                    }
                    _ => {
                        // Absolute mode - copy 'value' pixels literally
                        let copy_count = value as usize;
                        let bytes_to_read = copy_count.div_ceil(2); // Two 4-bit pixels per byte
                                                                    // Pad to word (16-bit) boundary
                        let padded_bytes = if bytes_to_read % 2 != 0 {
                            bytes_to_read + 1
                        } else {
                            bytes_to_read
                        };

                        if i + 2 + padded_bytes <= data.len() {
                            for j in 0..copy_count {
                                if pixel_index < pixels.len() {
                                    let byte_idx = j / 2;
                                    let pixel_byte = data[i + 2 + byte_idx];
                                    let pixel_value = if j % 2 == 0 {
                                        (pixel_byte >> 4) & 0x0F // High nibble first
                                    } else {
                                        pixel_byte & 0x0F // Low nibble second
                                    };
                                    pixels[pixel_index] = pixel_value;
                                    pixel_index += 1;
                                }
                            }
                            i += 2 + padded_bytes;
                        } else {
                            // Not enough data - this is an error in the RLE stream
                            return Err(BmpError::RleDecodingFailed);
                        }
                    }
                }
            } else {
                // Encoded mode - repeat the pixel pair 'count' times
                // Each byte contains two 4-bit pixels: high nibble first, then low nibble
                let high_nibble = (value >> 4) & 0x0F;
                let low_nibble = value & 0x0F;

                for _ in 0..(count / 2) {
                    // Output the pixel pair (high nibble, then low nibble)
                    if pixel_index < pixels.len() {
                        pixels[pixel_index] = high_nibble;
                        pixel_index += 1;
                    }
                    if pixel_index < pixels.len() {
                        pixels[pixel_index] = low_nibble;
                        pixel_index += 1;
                    }
                }

                // Handle odd count by outputting just the high nibble
                if count % 2 == 1 && pixel_index < pixels.len() {
                    pixels[pixel_index] = high_nibble;
                    pixel_index += 1;
                }

                i += 2;
            }
        }

        // Check if we decoded the expected number of pixels
        let expected_pixels = (width * height) as usize;
        if pixel_index < expected_pixels {
            return Err(BmpError::InsufficientData);
        }

        // Convert indexed pixels to RGB using palette
        self.convert_indexed_to_rgb(&pixels, width, height, is_top_down)
    }

    /// Check if all BITFIELDS masks are zero (indicating fallback to uncompressed behavior)
    fn all_masks_zero(&self) -> bool {
        self.red_mask.unwrap_or(0) == 0
            && self.green_mask.unwrap_or(0) == 0
            && self.blue_mask.unwrap_or(0) == 0
            && self.alpha_mask.unwrap_or(0) == 0
    }

    fn decompress_bitfields(
        &self,
        data: &[u8],
        width: u32,
        height: u32,
        bits_per_pixel: u16,
    ) -> Result<Vec<u8>, BmpError> {
        // Chromium's BITFIELDS approach:
        // 1. Calculate bit shifts and lookup tables for each mask
        // 2. Extract pixel values using masks and shifts
        // 3. Scale to 8-bit per component using lookup tables if needed

        let bytes_per_pixel = (bits_per_pixel / 8) as usize;
        let row_bytes = ((width as usize) * bytes_per_pixel + 3) & !3; // 32-bit aligned
        let required_size = row_bytes * height as usize;

        if data.len() < required_size {
            return Err(BmpError::InsufficientData);
        }

        // Process each mask to get shifts and lookup tables
        struct MaskInfo {
            mask: u32,
            shift_right: u32,
            lookup_table: Option<Vec<u8>>,
        }

        let process_mask = |mask: u32| -> Result<MaskInfo, BmpError> {
            if mask == 0 {
                return Ok(MaskInfo {
                    mask: 0,
                    shift_right: 0,
                    lookup_table: None,
                });
            }

            // Trim mask to bit depth
            let mask = if bits_per_pixel < 32 {
                mask & ((1u32 << bits_per_pixel) - 1)
            } else {
                mask
            };

            // Count offset (trailing zeros)
            let mut temp_mask = mask;
            let mut shift_right = 0u32;
            while temp_mask != 0 && (temp_mask & 1) == 0 {
                temp_mask >>= 1;
                shift_right += 1;
            }

            // Count mask size (number of contiguous 1s)
            let mut num_bits = 0u32;
            while temp_mask & 1 != 0 {
                temp_mask >>= 1;
                num_bits += 1;
            }

            // Mask must be contiguous
            if temp_mask != 0 {
                return Err(BmpError::InvalidData("Non-contiguous bitmask".to_string()));
            }

            // Scale to 8 bits per channel like Chromium
            let (final_shift, lookup_table) = if num_bits >= 8 {
                (shift_right + (num_bits - 8), None)
            } else {
                // Create n-bit to 8-bit lookup table
                // Chromium uses nBitTo8BitlookupTable
                let table_size = 1usize << num_bits;
                let mut table = vec![0u8; table_size];
                for (i, item) in table.iter_mut().enumerate().take(table_size) {
                    // Scale: multiply by 255 and divide by max value
                    *item = ((i * 255) / (table_size - 1)) as u8;
                }
                (shift_right, Some(table))
            };

            Ok(MaskInfo {
                mask,
                shift_right: final_shift,
                lookup_table,
            })
        };

        let red_info = process_mask(self.red_mask.unwrap_or(0))?;
        let green_info = process_mask(self.green_mask.unwrap_or(0))?;
        let blue_info = process_mask(self.blue_mask.unwrap_or(0))?;
        let _alpha_info = process_mask(self.alpha_mask.unwrap_or(0))?; // Chromium has complex alpha handling

        // Helper to extract component from pixel
        let get_component = |pixel: u32, info: &MaskInfo| -> u8 {
            if info.mask == 0 {
                return ALPHA_DEFAULT;
            }
            let value = ((pixel & info.mask) >> info.shift_right) as u8;
            if let Some(ref table) = info.lookup_table {
                table[value as usize]
            } else {
                value
            }
        };

        // Decode pixels
        let mut decoded_pixels = Vec::with_capacity((width * height * 3) as usize);

        // Read pixels row by row (bottom-up)
        for y in (0..height).rev() {
            let row_offset = (y as usize) * row_bytes;
            for x in 0..width as usize {
                let pixel_offset = row_offset + x * bytes_per_pixel;

                // Read pixel value (little-endian)
                let pixel = match bytes_per_pixel {
                    2 => u32::from(u16::from_le_bytes([
                        data[pixel_offset],
                        data[pixel_offset + 1],
                    ])),
                    3 => u32::from_le_bytes([
                        data[pixel_offset],
                        data[pixel_offset + 1],
                        data[pixel_offset + 2],
                        0,
                    ]),
                    4 => u32::from_le_bytes([
                        data[pixel_offset],
                        data[pixel_offset + 1],
                        data[pixel_offset + 2],
                        data[pixel_offset + 3],
                    ]),
                    _ => {
                        return Err(BmpError::UnsupportedFeature(format!(
                            "BITFIELDS with {} bytes per pixel",
                            bytes_per_pixel
                        )))
                    }
                };

                // Extract color components using masks
                let r = get_component(pixel, &red_info);
                let g = get_component(pixel, &green_info);
                let b = get_component(pixel, &blue_info);

                // For now, ignore alpha channel for RGB output
                //
                decoded_pixels.push(r);
                decoded_pixels.push(g);
                decoded_pixels.push(b);
            }
        }

        Ok(decoded_pixels)
    }

    /// Convert indexed pixels to RGB using the color table
    fn convert_indexed_to_rgb(
        &self,
        pixels: &[u8],
        width: u32,
        height: u32,
        is_top_down: bool,
    ) -> Result<Vec<u8>, BmpError> {
        // Ensure we have a color table
        if self.color_table.is_empty() {
            // Create a default grayscale palette as fallback
            let mut default_palette = Vec::new();
            for i in 0..256 {
                default_palette.push(RgbTriple {
                    r: i as u8,
                    g: i as u8,
                    b: i as u8,
                });
            }
            return self.convert_indexed_to_rgb_with_palette(
                pixels,
                width,
                height,
                is_top_down,
                &default_palette,
            );
        }

        self.convert_indexed_to_rgb_with_palette(
            pixels,
            width,
            height,
            is_top_down,
            &self.color_table,
        )
    }

    /// Convert indexed pixels to RGB using a specific palette
    fn convert_indexed_to_rgb_with_palette(
        &self,
        pixels: &[u8],
        width: u32,
        height: u32,
        is_top_down: bool,
        palette: &[RgbTriple],
    ) -> Result<Vec<u8>, BmpError> {
        let mut result = Vec::with_capacity((width * height * 3) as usize);

        for y in 0..height {
            for x in 0..width {
                let row_index = if is_top_down { y } else { height - 1 - y };
                let pixel_idx = (row_index * width + x) as usize;

                if pixel_idx < pixels.len() {
                    let color_index = pixels[pixel_idx] as usize;
                    if color_index < palette.len() {
                        let color = palette[color_index];
                        result.push(color.r);
                        result.push(color.g);
                        result.push(color.b);
                    } else {
                        // Invalid palette index - use black
                        result.push(0);
                        result.push(0);
                        result.push(0);
                    }
                } else {
                    // Missing pixel data - use black
                    result.push(0);
                    result.push(0);
                    result.push(0);
                }
            }
        }

        Ok(result)
    }
}
