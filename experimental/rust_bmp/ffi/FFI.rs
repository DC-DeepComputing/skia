// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for BMP decoder using the image-rs crate.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

mod bmp_types;

// No `use image::...` nor `use ffi::...` because we want the code to explicitly
// spell out if it means `ffi::ColorType` vs `image::ColorType` (or `BmpDecoder`
// vs `image::codecs::bmp::BmpDecoder`).

#[cxx::bridge(namespace = "rust_bmp")]
mod ffi {
    #[derive(Debug, Clone, Copy)]
    enum DecodingResult {
        Success,
        FormatError,
        ParameterError,
        UnsupportedFeature,
        IncompleteInput,
        MemoryError,
        OtherError,
    }

    /// Output pixel format from the decoder.
    /// 
    /// Note: BGR/BGRA variants are included because BMP natively uses BGR byte order,
    /// though the current image-rs implementation converts to RGB during decoding.
    /// These variants provide flexibility for future optimizations or alternative decoders.
    #[derive(Debug, Clone, Copy)]
    enum ColorType {
        Unknown = 0,
        Grayscale = 1,
        Rgb = 2,
        Rgba = 3,
        Bgr = 4,
        Bgra = 5,
    }

    #[derive(Debug, Clone, Copy)]
    enum AlphaType {
        Unknown = 0,
        Opaque = 1,
        Premul = 2,
        Unpremul = 3,
    }

    struct DecodeInfoResult {
        success: bool,
        width: u32,
        height: u32,
        color_type: ColorType,
        alpha_type: AlphaType,
    }

    struct DecodePixelResult {
        success: bool,
        error_is_incomplete: bool,
        pixels: Vec<u8>,
        width: u32,
        height: u32,
        bytes_per_pixel: u32,
        data_is_incomplete: bool,
    }

    // C++ interface for streaming
    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        type BmpDecoder;

        fn create_decoder() -> Box<BmpDecoder>;
        fn decode_info(self: &mut BmpDecoder, data: &[u8]) -> DecodeInfoResult;
        fn decode_pixels(self: &mut BmpDecoder, data: &[u8]) -> DecodePixelResult;
        fn is_bmp_data(data: &[u8]) -> bool;

        // Streaming API
        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color_type(self: &Reader) -> ColorType;
        fn alpha_type(self: &Reader) -> AlphaType;
        fn bottom_up(self: &Reader) -> bool;
        fn next_row(self: &mut Reader, buffer: &mut [u8]) -> DecodingResult;
        fn reset_decode_state(self: &mut Reader);
    }
}

pub use ffi::*;

pub struct BmpDecoder {
    // Store last decoded info for the legacy API
    width: u32,
    height: u32,
    color_type: ColorType,
    alpha_type: AlphaType,
}

pub fn create_decoder() -> Box<BmpDecoder> {
    Box::new(BmpDecoder {
        width: 0,
        height: 0,
        color_type: ColorType::Unknown,
        alpha_type: AlphaType::Unknown,
    })
}

impl BmpDecoder {
    pub fn decode_info(&mut self, data: &[u8]) -> DecodeInfoResult {
        use std::io::Cursor;
        
        let mut cursor = Cursor::new(data);
        
        match image::codecs::bmp::BmpDecoder::new(&mut cursor) {
            Ok(decoder) => {
                let (width, height) = image::ImageDecoder::dimensions(&decoder);
                let image_color_type = image::ImageDecoder::color_type(&decoder);
                
                // Map image-rs color type to our FFI color type
                let (color_type, alpha_type) = match image_color_type {
                    image::ColorType::Rgb8 => (ColorType::Rgb, AlphaType::Opaque),
                    image::ColorType::Rgba8 => (ColorType::Rgba, AlphaType::Unpremul),
                    image::ColorType::L8 => (ColorType::Grayscale, AlphaType::Opaque),
                    _ => (ColorType::Unknown, AlphaType::Unknown),
                };
                
                // Store for later use in decode_pixels
                self.width = width;
                self.height = height;
                self.color_type = color_type;
                self.alpha_type = alpha_type;
                
                DecodeInfoResult {
                    success: true,
                    width,
                    height,
                    color_type,
                    alpha_type,
                }
            }
            Err(_) => DecodeInfoResult {
                success: false,
                width: 0,
                height: 0,
                color_type: ColorType::Unknown,
                alpha_type: AlphaType::Unknown,
            },
        }
    }

    pub fn decode_pixels(&mut self, data: &[u8]) -> DecodePixelResult {
        use std::io::Cursor;
        
        let mut cursor = Cursor::new(data);
        
        match image::codecs::bmp::BmpDecoder::new(&mut cursor) {
            Ok(decoder) => {
                let (width, height) = image::ImageDecoder::dimensions(&decoder);
                let total_bytes = image::ImageDecoder::total_bytes(&decoder) as usize;
                let bytes_per_pixel = match image::ImageDecoder::color_type(&decoder) {
                    image::ColorType::Rgb8 => 3,
                    image::ColorType::Rgba8 => 4,
                    image::ColorType::L8 => 1,
                    _ => 3,
                };
                let mut pixels = vec![0u8; total_bytes];
                
                match image::ImageDecoder::read_image(decoder, &mut pixels) {
                    Ok(_) => {
                        DecodePixelResult {
                            success: true,
                            error_is_incomplete: false,
                            pixels,
                            width,
                            height,
                            bytes_per_pixel,
                            data_is_incomplete: false,
                        }
                    }
                    Err(_) => DecodePixelResult {
                        success: false,
                        error_is_incomplete: false,
                        pixels: Vec::new(),
                        width: 0,
                        height: 0,
                        bytes_per_pixel: 0,
                        data_is_incomplete: false,
                    },
                }
            }
            Err(_) => DecodePixelResult {
                success: false,
                error_is_incomplete: false,
                pixels: Vec::new(),
                width: 0,
                height: 0,
                bytes_per_pixel: 0,
                data_is_incomplete: false,
            },
        }
    }
}

/// Check if data starts with BMP signature
pub fn is_bmp_data(data: &[u8]) -> bool {
    // BMP files start with "BM" (0x42 0x4D)
    data.len() >= 2 && data[0] == 0x42 && data[1] == 0x4D
}

// Streaming Reader API

/// Result wrapper for Reader creation
pub struct ResultOfReader {
    result: Result<Reader, bmp_types::BmpError>,
}

impl ResultOfReader {
    pub fn err(&self) -> DecodingResult {
        match &self.result {
            Ok(_) => DecodingResult::Success,
            Err(bmp_types::BmpError::InvalidHeader) => DecodingResult::FormatError,
            Err(bmp_types::BmpError::InsufficientData) => DecodingResult::IncompleteInput,
            Err(bmp_types::BmpError::UnsupportedFeature(_)) => DecodingResult::UnsupportedFeature,
            Err(bmp_types::BmpError::InvalidData(_)) => DecodingResult::FormatError,
            Err(bmp_types::BmpError::IoError(_)) => DecodingResult::OtherError,
            Err(bmp_types::BmpError::FormatError(_)) => DecodingResult::FormatError,
        }
    }

    pub fn unwrap(&mut self) -> Box<Reader> {
        match std::mem::replace(&mut self.result, Err(bmp_types::BmpError::InvalidHeader)) {
            Ok(reader) => Box::new(reader),
            Err(_) => panic!("Called unwrap on an error ResultOfReader"),
        }
    }
}

/// Streaming BMP reader using image-rs
///
/// Note: Unlike PNG, the image-rs BMP decoder doesn't support true row-by-row streaming.
/// The `ImageDecoder::read_image()` method decodes the entire image at once. Therefore,
/// this Reader stores the complete decoded image in memory and provides a streaming
/// interface via `next_row()` for compatibility with Skia's codec architecture.
///
/// This approach is acceptable because:
/// 1. BMP files are typically smaller than other formats
/// 2. The image-rs API doesn't provide incremental row decoding for BMP
/// 3. It maintains API compatibility with other Skia decoders
pub struct Reader {
    // Store the decoded image data in memory
    pixels: Vec<u8>,
    width: u32,
    height: u32,
    color_type: ColorType,
    alpha_type: AlphaType,
    bytes_per_pixel: u32,
    current_row: u32,
    bottom_up: bool,
}

impl Reader {
    fn new(mut input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, bmp_types::BmpError> {
        use std::io::{Cursor, Read};
        
        // TODO(https://crbug.com/382556889): Improve incremental/streaming decode support.
        //
        // Current limitation: We read the entire stream into memory upfront because
        // image-rs BmpDecoder requires a seekable input (std::io::Seek). This means:
        // - If the stream has incomplete data, we fail during MakeFromStream
        // - The caller (e.g., Blink's SkiaImageDecoderBase) must retry from scratch
        //
        // For better streaming support (matching SkPngRustCodec behavior):
        // - Could buffer data incrementally and retry BmpDecoder::new() on more data
        // - Would need to handle partial header reads gracefully
        // - Alternative: Implement custom BMP header parser that works with streaming
        //
        // Note: The image-rs API doesn't support true row-by-row streaming anyway
        // (see Reader documentation above), so the benefit may be limited to avoiding
        // re-parsing headers when more data arrives.
        
        // Read entire stream into memory. The image-rs BMP decoder requires
        // a seekable input (like Cursor), so we load the full data first.
        // This also simplifies error handling across the FFI boundary.
        let mut data = Vec::new();
        {
            // input.as_mut() already returns Pin<&mut SkStreamAdapter>
            // which implements Read via the skia_rust_common trait impl
            input.as_mut()
                .expect("input should not be null")
                .read_to_end(&mut data)
                .map_err(|_| bmp_types::BmpError::InsufficientData)?;
        }
        
        // Parse BMP header to determine if bottom-up or top-down
        // BMP file header: 14 bytes
        // DIB header starts at offset 14
        // Height is at offset 14+8 = 22 (as signed i32)
        let bottom_up = if data.len() >= 26 {
            // Read height as signed i32 at offset 22
            let height_bytes = [data[22], data[23], data[24], data[25]];
            let height_signed = i32::from_le_bytes(height_bytes);
            // Positive height = bottom-up, negative = top-down
            height_signed > 0
        } else {
            // If can't read header, assume bottom-up (most common)
            true
        };
        
        // Create a Cursor for image-rs (requires Seek + Read)
        let mut cursor = Cursor::new(&data);
        
        // Create image-rs BMP decoder
        let decoder = image::codecs::bmp::BmpDecoder::new(&mut cursor)
            .map_err(|_| bmp_types::BmpError::InvalidHeader)?;
        
        // Get image dimensions
        let (width, height) = image::ImageDecoder::dimensions(&decoder);
        
        // Get color type from image-rs
        let image_color_type = image::ImageDecoder::color_type(&decoder);
        
        // Map image-rs color type to our FFI color type.
        // Note: The image-rs BMP decoder internally converts BGR→RGB during decoding,
        // so we report RGB/RGBA even though BMP files natively use BGR byte order.
        // This is an implementation detail - our ColorType enum supports both RGB and BGR
        // to allow flexibility for future decoders or optimizations.
        let (color_type, alpha_type, bytes_per_pixel) = match image_color_type {
            image::ColorType::Rgb8 => (ColorType::Rgb, AlphaType::Opaque, 3),
            image::ColorType::Rgba8 => (ColorType::Rgba, AlphaType::Unpremul, 4),
            image::ColorType::L8 => (ColorType::Grayscale, AlphaType::Opaque, 1),
            _ => {
                return Err(bmp_types::BmpError::UnsupportedFeature(
                    format!("Unsupported color type: {:?}", image_color_type)
                ));
            }
        };
        
        // Decode entire image into memory.
        // Note: image-rs BmpDecoder doesn't support incremental row decoding.
        // The entire image must be decoded at once via read_image().
        let total_bytes = image::ImageDecoder::total_bytes(&decoder) as usize;
        let mut pixels = vec![0u8; total_bytes];
        image::ImageDecoder::read_image(decoder, &mut pixels)
            .map_err(|e| {
                bmp_types::BmpError::InvalidData(format!("Failed to decode image: {:?}", e))
            })?;
        
        Ok(Reader {
            pixels,
            width,
            height,
            color_type,
            alpha_type,
            bytes_per_pixel,
            current_row: 0,
            bottom_up,
        })
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color_type(&self) -> ColorType {
        self.color_type
    }

    pub fn alpha_type(&self) -> AlphaType {
        self.alpha_type
    }

    pub fn bottom_up(&self) -> bool {
        self.bottom_up
    }

    pub fn reset_decode_state(&mut self) {
        self.current_row = 0;
    }

    pub fn next_row(&mut self, buffer: &mut [u8]) -> DecodingResult {
        if self.current_row >= self.height {
            return DecodingResult::Success; // No more rows
        }

        let row_bytes = (self.width * self.bytes_per_pixel) as usize;
        
        if buffer.len() < row_bytes {
            return DecodingResult::ParameterError;
        }

        // Copy one row from decoded pixels
        let row_offset = (self.current_row as usize) * row_bytes;
        buffer[..row_bytes].copy_from_slice(&self.pixels[row_offset..row_offset + row_bytes]);
        
        self.current_row += 1;
        DecodingResult::Success
    }
}

/// Creates a new streaming BMP reader
pub fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<ResultOfReader> {
    let result = Reader::new(input);
    Box::new(ResultOfReader { result })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_bmp_data() {
        // Valid BMP signature
        let valid_bmp = vec![0x42, 0x4D, 0x00, 0x00];
        assert!(is_bmp_data(&valid_bmp));

        // Invalid signature
        let invalid = vec![0x89, 0x50, 0x4E, 0x47]; // PNG signature
        assert!(!is_bmp_data(&invalid));

        // Too short
        let too_short = vec![0x42];
        assert!(!is_bmp_data(&too_short));

        // Empty
        assert!(!is_bmp_data(&[]));
    }
}
