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
        fn is_bottom_up(self: &Reader) -> bool;
        fn color(self: &Reader) -> BmpColor;
        fn alpha(self: &Reader) -> BmpAlpha;
        fn metadata_loaded(self: &Reader) -> bool;

        /// Attempt to read and parse BMP header metadata from the stream.
        /// Returns Success if metadata was loaded, IncompleteInput if more data is needed.
        fn read_metadata(self: &mut Reader) -> DecodingResult;

        /// Attempt to read all image pixel data from the stream into internal buffer.
        /// Returns Success if all data was read, IncompleteInput if more data is needed.
        /// After success, use get_image_data() to access the decoded pixels.
        fn read_image_data(self: &mut Reader) -> DecodingResult;

        /// Check if all image data has been loaded into the internal buffer.
        fn image_data_loaded(self: &Reader) -> bool;

        /// Get the number of fully decoded rows currently available in the buffer.
        /// This can be called after read_image_data() returns IncompleteInput to
        /// determine how many rows are available for progressive rendering.
        fn rows_decoded(self: &Reader) -> u32;

        /// Get pointer to entire image buffer.
        /// Returns empty slice if data not loaded.
        fn get_image_data(self: &Reader) -> &[u8];

        /// Get the number of bytes per row.
        fn row_bytes(self: &Reader) -> u32;

        /// Reset internal state for re-decoding (clears buffered image data).
        fn reset_decode_state(self: &mut Reader);

        // ICC profile support
        fn icc_profile(self: &mut Reader) -> Vec<u8>;
    }
}

pub use ffi::*;

#[derive(Debug)]
enum BmpError {
    InsufficientData,
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
            image::ImageError::IoError(io_err) => {
                // Check if the underlying IO error is UnexpectedEof, which indicates
                // insufficient data in streaming mode
                if io_err.kind() == std::io::ErrorKind::UnexpectedEof {
                    BmpError::InsufficientData
                } else {
                    BmpError::IoError
                }
            }
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

/// BMP decoder with resumable/streaming support.
/// Both read_metadata() and read_image_data() can be retried on IncompleteInput.
pub struct Reader {
    decoder: Option<
        image::codecs::bmp::BmpDecoder<std::io::BufReader<cxx::UniquePtr<ffi::SkStreamAdapter>>>,
    >,
    metadata_loaded: bool,
    width: u32,
    height: u32,
    is_bottom_up: bool,
    color: BmpColor,
    alpha: BmpAlpha,
    bytes_per_pixel: u32,
    /// Buffered decoded image data (all rows)
    image_data: Vec<u8>,
    image_data_loaded: bool,
}
impl Reader {
    fn new(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, BmpError> {
        use std::io::BufReader;

        // Use a larger buffer (64KB) to reduce syscall overhead for large images.
        // Default BufReader uses 8KB which causes many small reads.
        const BUFFER_SIZE: usize = 64 * 1024;
        let buffered = BufReader::with_capacity(BUFFER_SIZE, input);

        // new_resumable() creates the decoder without reading any data from the stream.
        // The caller must call read_metadata() to read the BMP headers.
        // This constructor is infallible - it just wraps the reader.
        let decoder = image::codecs::bmp::BmpDecoder::new_resumable(buffered);

        Ok(Reader {
            decoder: Some(decoder),
            metadata_loaded: false,
            width: 0,
            height: 0,
            is_bottom_up: true,
            color: BmpColor::RGB,
            alpha: BmpAlpha::Opaque,
            bytes_per_pixel: 0,
            image_data: Vec::new(),
            image_data_loaded: false,
        })
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn is_bottom_up(&self) -> bool {
        self.is_bottom_up
    }

    pub fn color(&self) -> BmpColor {
        self.color
    }

    pub fn alpha(&self) -> BmpAlpha {
        self.alpha
    }

    pub fn metadata_loaded(&self) -> bool {
        self.metadata_loaded
    }

    pub fn read_metadata(&mut self) -> DecodingResult {
        if self.metadata_loaded {
            return DecodingResult::Success;
        }

        let decoder = match &mut self.decoder {
            Some(d) => d,
            None => return DecodingResult::FormatError,
        };

        match decoder.read_metadata() {
            Ok(()) => {}
            Err(e) => {
                return match BmpError::from(e) {
                    BmpError::InsufficientData => DecodingResult::IncompleteInput,
                    BmpError::UnsupportedFeature => DecodingResult::UnsupportedFeature,
                    _ => DecodingResult::FormatError,
                };
            }
        }

        use image::ImageDecoder;
        let (width, height) = decoder.dimensions();
        let image_color_type = decoder.color_type();

        let (color, alpha, bytes_per_pixel) = match image_color_type {
            image::ColorType::Rgb8 => (BmpColor::RGB, BmpAlpha::Opaque, 3),
            image::ColorType::Rgba8 => (BmpColor::RGBA, BmpAlpha::Unpremul, 4),
            _ => {
                return DecodingResult::UnsupportedFeature;
            }
        };

        self.width = width;
        self.height = height;
        self.color = color;
        self.alpha = alpha;
        self.bytes_per_pixel = bytes_per_pixel;
        self.is_bottom_up = false;
        self.metadata_loaded = true;

        DecodingResult::Success
    }

    pub fn read_image_data(&mut self) -> DecodingResult {
        if self.image_data_loaded {
            return DecodingResult::Success;
        }

        if !self.metadata_loaded {
            return DecodingResult::ParameterError;
        }

        let decoder = match self.decoder.as_mut() {
            Some(d) => d,
            None => return DecodingResult::FormatError,
        };

        let row_bytes = (self.width as usize) * (self.bytes_per_pixel as usize);
        let total_bytes = row_bytes * (self.height as usize);

        // Allocate zero-initialized buffer for the image data.
        // Zero-initialization ensures that any unwritten bytes (e.g., on partial
        // decode) are valid zeros rather than garbage.
        if self.image_data.len() < total_bytes {
            self.image_data = vec![0u8; total_bytes];
        }

        match decoder.read_image_data(&mut self.image_data) {
            Ok(()) => {
                self.image_data_loaded = true;
                DecodingResult::Success
            }
            Err(e) => {
                // On IncompleteInput, keep the partially decoded data in the buffer.
                // The decoder writes row data as it decodes, and rows_decoded() tells
                // us how many rows are complete. We keep the full buffer allocated
                // but get_image_data() will only return the valid portion.
                match BmpError::from(e) {
                    BmpError::InsufficientData => DecodingResult::IncompleteInput,
                    BmpError::UnsupportedFeature => {
                        self.image_data.clear();
                        DecodingResult::UnsupportedFeature
                    }
                    _ => {
                        self.image_data.clear();
                        DecodingResult::FormatError
                    }
                }
            }
        }
    }

    pub fn image_data_loaded(&self) -> bool {
        self.image_data_loaded
    }

    pub fn rows_decoded(&self) -> u32 {
        if let Some(decoder) = &self.decoder {
            decoder.rows_decoded()
        } else {
            0
        }
    }

    /// Get pointer to the decoded image data buffer.
    /// Returns a slice containing all fully decoded rows.
    /// For complete decodes, this is the entire image.
    /// For partial decodes (after IncompleteInput), this contains only the
    /// rows that have been fully decoded so far.
    pub fn get_image_data(&self) -> &[u8] {
        if self.image_data.is_empty() {
            return &[];
        }

        // If fully loaded, return the entire buffer
        if self.image_data_loaded {
            return &self.image_data;
        }

        // For partial decodes, return only the valid portion
        let rows = self.rows_decoded() as usize;
        let row_bytes = (self.width as usize) * (self.bytes_per_pixel as usize);
        let valid_bytes = rows * row_bytes;

        // Safety: valid_bytes is at most height * row_bytes which equals image_data.len()
        &self.image_data[..valid_bytes]
    }

    pub fn row_bytes(&self) -> u32 {
        self.width * self.bytes_per_pixel
    }

    pub fn reset_decode_state(&mut self) {
        self.image_data.clear();
        self.image_data_loaded = false;
    }

    pub fn icc_profile(&mut self) -> Vec<u8> {
        use image::ImageDecoder;
        match &mut self.decoder {
            Some(d) => match d.icc_profile() {
                Ok(Some(profile)) => profile,
                Ok(None) | Err(_) => Vec::new(),
            },
            None => Vec::new(),
        }
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
        // Valid BMP signature (BM)
        let valid_bmp = vec![b'B', b'M', 0x00, 0x00];
        assert!(is_bmp_data(&valid_bmp));

        // BA signature not yet supported by image crate
        let valid_ba = vec![b'B', b'A', 0x00, 0x00];
        assert!(!is_bmp_data(&valid_ba));

        // Invalid signature
        let invalid = vec![0x89, b'P', b'N', b'G'];
        assert!(!is_bmp_data(&invalid));

        // Too short
        let too_short = vec![b'B'];
        assert!(!is_bmp_data(&too_short));

        // Empty
        assert!(!is_bmp_data(&[]));
    }
}
