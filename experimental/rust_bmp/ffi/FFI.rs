// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for BMP decoder using the image-rs crate.

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

    /// BMP color type matching SkEncodedInfo::Color values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum BmpColor {
        RGB = 5,
        RGBA = 6,
        BGR = 7,
        BGRA = 9,
    }

    /// BMP alpha type matching SkEncodedInfo::Alpha values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum BmpAlpha {
        Opaque = 0,
        Unpremul = 1,
    }

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        fn is_bmp_data(data: &[u8]) -> bool;

        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> BmpColor;
        fn alpha(self: &Reader) -> BmpAlpha;
        fn next_row(self: &mut Reader, buffer: &mut [u8]) -> DecodingResult;
        fn reset_decode_state(self: &mut Reader);
        fn metadata_loaded(self: &Reader) -> bool;
        
        // Streaming/incremental decoding methods
        fn try_read_row(self: &mut Reader, buffer: &mut [u8]) -> DecodingResult;
        fn has_more_rows(self: &Reader) -> bool;
        fn current_row_index(self: &Reader) -> u32;
        
        // ICC profile support
        fn icc_profile(self: &mut Reader) -> Vec<u8>;
    }
}

pub use ffi::*;

#[derive(Debug)]
enum BmpError {
    InsufficientData,
    InvalidData,
    UnsupportedFeature,
    InvalidHeader,
    IoError,
    FormatError,
}

impl From<image::ImageError> for BmpError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => BmpError::FormatError,
            image::ImageError::Encoding(_) => BmpError::FormatError,
            image::ImageError::Parameter(_) => {
                panic!(
                    "Internal bug in how Skia Rust BMP calls image crate: {:?}",
                    err
                )
            }
            image::ImageError::Limits(_) => BmpError::UnsupportedFeature,
            image::ImageError::Unsupported(_) => BmpError::UnsupportedFeature,
            image::ImageError::IoError(_) => BmpError::IoError,
            image::ImageError::InsufficientData(_) => BmpError::InsufficientData,
        }
    }
}

impl From<std::io::Error> for BmpError {
    fn from(err: std::io::Error) -> Self {
        use std::io::ErrorKind;
        match err.kind() {
            ErrorKind::UnexpectedEof => BmpError::InsufficientData,
            _ => BmpError::IoError,
        }
    }
}

pub fn is_bmp_data(data: &[u8]) -> bool {
    // "IC", "PT", "CI", "CP", "BA" are not yet supported by image crate.
    data.len() >= 2 && data[0] == b'B' && data[1] == b'M'
}

pub struct ResultOfReader {
    result: Result<Reader, BmpError>,
}

impl ResultOfReader {
    pub fn err(&self) -> DecodingResult {
        match &self.result {
            Ok(_) => DecodingResult::Success,
            Err(BmpError::InvalidHeader) => DecodingResult::FormatError,
            Err(BmpError::InsufficientData) => DecodingResult::IncompleteInput,
            Err(BmpError::UnsupportedFeature) => DecodingResult::UnsupportedFeature,
            Err(BmpError::InvalidData) => DecodingResult::FormatError,
            Err(BmpError::IoError) => DecodingResult::OtherError,
            Err(BmpError::FormatError) => DecodingResult::FormatError,
        }
    }

    pub fn unwrap(&mut self) -> Box<Reader> {
        match std::mem::replace(&mut self.result, Err(BmpError::InvalidHeader)) {
            Ok(reader) => Box::new(reader),
            Err(_) => panic!("Called unwrap on an error ResultOfReader"),
        }
    }
}

/// BMP decoder with streaming support.
pub struct Reader {
    decoder: image::codecs::bmp::BmpDecoder<std::io::Cursor<Vec<u8>>>,
    metadata_loaded: bool,
    width: u32,
    height: u32,
    color: BmpColor,
    alpha: BmpAlpha,
    bytes_per_pixel: u32,
    current_row: u32,
}impl Reader {
    fn new(mut input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, BmpError> {
        use std::io::{Cursor, Read};

        let mut buffer = Vec::new();
        input
            .as_mut()
            .expect("input should not be null")
            .read_to_end(&mut buffer)
            .map_err(|_| BmpError::InsufficientData)?;

        let cursor = Cursor::new(buffer);
        let decoder = image::codecs::bmp::BmpDecoder::new_streaming(cursor)
            .map_err(BmpError::from)?;

        Ok(Reader {
            decoder,
            metadata_loaded: false,
            width: 0,
            height: 0,
            color: BmpColor::RGB,
            alpha: BmpAlpha::Opaque,
            bytes_per_pixel: 0,
            current_row: 0,
        })
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color(&self) -> BmpColor {
        self.color
    }

    pub fn alpha(&self) -> BmpAlpha {
        self.alpha
    }

    fn try_load_metadata(&mut self) -> Result<bool, BmpError> {
        if self.metadata_loaded {
            return Ok(true);
        }

        match self.decoder.try_read_metadata() {
            Ok(Some(())) => {
                let (width, height) = <image::codecs::bmp::BmpDecoder<_> as image::ImageDecoder>::dimensions(&self.decoder);
                let image_color_type = <image::codecs::bmp::BmpDecoder<_> as image::ImageDecoder>::color_type(&self.decoder);

                let (color, alpha, bytes_per_pixel) = match image_color_type {
                    image::ColorType::Rgb8 => (BmpColor::RGB, BmpAlpha::Opaque, 3),
                    image::ColorType::Rgba8 => (BmpColor::RGBA, BmpAlpha::Unpremul, 4),
                    _ => {
                        return Err(BmpError::UnsupportedFeature);
                    }
                };

                self.width = width;
                self.height = height;
                self.color = color;
                self.alpha = alpha;
                self.bytes_per_pixel = bytes_per_pixel;
                self.metadata_loaded = true;
                Ok(true)
            }
            Ok(None) => Ok(false),
            Err(e) => Err(BmpError::from(e)),
        }
    }

    pub fn reset_decode_state(&mut self) {
        self.current_row = 0;
        // If using streaming, we can't easily reset the decoder state,
        // so we just reset the row counter
    }

    pub fn next_row(&mut self, buffer: &mut [u8]) -> DecodingResult {
        self.try_read_row(buffer)
    }

    pub fn try_read_row(&mut self, buffer: &mut [u8]) -> DecodingResult {
        if !self.metadata_loaded {
            match self.try_load_metadata() {
                Ok(true) => {}
                Ok(false) => return DecodingResult::IncompleteInput,
                Err(BmpError::InsufficientData) => return DecodingResult::IncompleteInput,
                Err(BmpError::UnsupportedFeature) => return DecodingResult::UnsupportedFeature,
                Err(_) => return DecodingResult::FormatError,
            }
        }

        if self.current_row >= self.height {
            return DecodingResult::Success;
        }

        let row_bytes = (self.width * self.bytes_per_pixel) as usize;
        if buffer.len() < row_bytes {
            return DecodingResult::ParameterError;
        }

        match self.decoder.try_read_row(buffer) {
            Ok(Some(row_data)) => {
                if row_data.len() != row_bytes {
                    return DecodingResult::FormatError;
                }
                self.current_row += 1;
                DecodingResult::Success
            }
            Ok(None) => DecodingResult::IncompleteInput,
            Err(e) => match BmpError::from(e) {
                BmpError::InsufficientData => DecodingResult::IncompleteInput,
                BmpError::UnsupportedFeature => DecodingResult::UnsupportedFeature,
                _ => DecodingResult::FormatError,
            },
        }
    }

    pub fn has_more_rows(&self) -> bool {
        self.current_row < self.height
    }

    pub fn current_row_index(&self) -> u32 {
        self.current_row
    }

    pub fn icc_profile(&mut self) -> Vec<u8> {
        match <image::codecs::bmp::BmpDecoder<_> as image::ImageDecoder>::icc_profile(&mut self.decoder) {
            Ok(Some(profile)) => profile,
            Ok(None) | Err(_) => Vec::new(),
        }
    }

    pub fn metadata_loaded(&self) -> bool {
        self.metadata_loaded
    }
}

pub fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<ResultOfReader> {
    let result = Reader::new(input);
    Box::new(ResultOfReader { result })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_bmp_data() {
        let valid_bmp = vec![b'B', b'M', 0x00, 0x00];
        assert!(is_bmp_data(&valid_bmp));

        let valid_ba = vec![b'B', b'A', 0x00, 0x00];
        assert!(!is_bmp_data(&valid_ba));

        let invalid = vec![0x89, b'P', b'N', b'G'];
        assert!(!is_bmp_data(&invalid));

        let too_short = vec![b'B'];
        assert!(!is_bmp_data(&too_short));

        assert!(!is_bmp_data(&[]));
    }
}
