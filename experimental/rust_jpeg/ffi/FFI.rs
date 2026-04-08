// Copyright 2026 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for JPEG decoder and encoder.
//!
//! Decoder: Uses the zune-jpeg crate for JPEG decoding (baseline + progressive).
//! Incremental decoding is structurally supported: both read_metadata() and
//! read_image_data() accumulate data from the stream on each call and return
//! IncompleteInput when insufficient data is available.  Note that zune-jpeg
//! buffers the full compressed stream before decoding, so IncompleteInput on
//! image data will only occur when the stream hits EOF mid-read; it will not
//! return IncompleteInput for partially-written rows.
//!
//! Encoder: Uses the jpeg-encoder crate's pure-Rust JPEG encoder.

#[cxx::bridge(namespace = "rust_jpeg")]
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

    /// JPEG color type matching SkEncodedInfo::Color values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum JpegColor {
        Grayscale = 0,
        RGB = 5,
    }

    /// JPEG alpha type matching SkEncodedInfo::Alpha values.
    /// Static assertions in C++ validate these match at compile time.
    #[repr(i32)]
    #[derive(Debug, Clone, Copy, PartialEq)]
    enum JpegAlpha {
        Opaque = 0,
    }

    /// Information about decoded rows.
    struct DecodedRowsInfo {
        /// The destination row index where the buffer should start being copied.
        dst_row_start: u32,
        row_count: u32,
    }

    // Encoder types
    #[derive(Debug, Clone, Copy)]
    enum EncodingResult {
        Success,
        ParameterError,
        OtherError,
    }

    #[derive(Debug, Clone, Copy)]
    enum JpegEncodeColor {
        RGB,
        RGBA,
        BGRA,
        Grayscale,
    }

    /// Alpha handling for JPEG encoding (JPEG is always opaque).
    #[derive(Debug, Clone, Copy)]
    enum JpegEncodeAlpha {
        /// Ignore alpha channel entirely.
        Ignore,
        /// Blend onto black background before encoding.
        BlendOnBlack,
    }

    unsafe extern "C++" {
        include!("rust/common/SkStreamAdapter.h");

        #[namespace = "rust::stream"]
        type SkStreamAdapter = skia_rust_common::SkStreamAdapter;
    }

    extern "Rust" {
        fn is_jpeg_data(data: &[u8]) -> bool;

        fn new_reader(input: UniquePtr<SkStreamAdapter>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn width(self: &Reader) -> u32;
        fn height(self: &Reader) -> u32;
        fn color(self: &Reader) -> JpegColor;
        fn alpha(self: &Reader) -> JpegAlpha;
        fn metadata_loaded(self: &Reader) -> bool;

        /// Attempt to read and parse JPEG header metadata from the stream.
        /// Returns IncompleteInput if not enough data is available yet.
        fn read_metadata(self: &mut Reader) -> DecodingResult;

        /// Attempt to read all image pixel data from the stream into internal buffer.
        /// Returns IncompleteInput if not enough data is available yet.
        fn read_image_data(self: &mut Reader) -> DecodingResult;

        fn image_data_loaded(self: &Reader) -> bool;

        /// Get info about NEW rows since last call.
        ///
        /// # Safety
        /// CXX requires `unsafe` for functions with explicit lifetimes.
        unsafe fn get_next_rows<'a>(self: &'a mut Reader, buffer: &mut &'a [u8])
            -> DecodedRowsInfo;

        fn row_bytes(self: &Reader) -> u32;

        fn reset_decode_state(self: &mut Reader);

        // Metadata extraction
        fn icc_profile(self: &mut Reader) -> Vec<u8>;
        fn exif_data(self: &mut Reader) -> Vec<u8>;

        // Encoder
        fn encode_jpeg(
            pixels: &[u8],
            width: u32,
            height: u32,
            row_bytes: u32,
            color_type: JpegEncodeColor,
            alpha_option: JpegEncodeAlpha,
            quality: u32,
            output: &mut Vec<u8>,
        ) -> EncodingResult;
    }
}

pub use ffi::*;

use zune_core::bytestream::ZCursor;
use zune_core::colorspace::ColorSpace as ZuneColorSpace;

const JPEG_SOI: u8 = 0xD8;

/// Read all available bytes from a SkStreamAdapter.
/// Does not block — reads whatever is available.
fn read_available_from_stream(
    input: &mut cxx::UniquePtr<ffi::SkStreamAdapter>,
) -> Result<Vec<u8>, ()> {
    use std::io::Read;
    let mut data = Vec::new();
    match input.as_mut().unwrap().read_to_end(&mut data) {
        Ok(_) => Ok(data),
        Err(_) => {
            if data.is_empty() {
                Err(())
            } else {
                Ok(data)
            }
        }
    }
}

/// Map zune-jpeg errors to our FFI DecodingResult.
fn map_zune_error(err: &zune_jpeg::errors::DecodeErrors) -> DecodingResult {
    use zune_jpeg::errors::DecodeErrors::*;
    match err {
        Unsupported(_) => DecodingResult::UnsupportedFeature,
        LargeDimensions(_) => DecodingResult::ParameterError,
        _ => DecodingResult::FormatError,
    }
}

/// Map a zune-core ColorSpace to our FFI JpegColor.
fn map_color_space(cs: ZuneColorSpace) -> (JpegColor, u32) {
    match cs {
        ZuneColorSpace::Luma | ZuneColorSpace::LumaA => (JpegColor::Grayscale, 1),
        _ => (JpegColor::RGB, 3),
    }
}

pub fn is_jpeg_data(data: &[u8]) -> bool {
    data.len() >= 2 && data[0] == 0xFF && data[1] == JPEG_SOI
}

pub struct ResultOfReader {
    result: Result<Reader, DecodingResult>,
}

impl ResultOfReader {
    pub fn err(&self) -> DecodingResult {
        match &self.result {
            Ok(_) => DecodingResult::Success,
            Err(e) => *e,
        }
    }

    pub fn unwrap(&mut self) -> Box<Reader> {
        match std::mem::replace(&mut self.result, Err(DecodingResult::FormatError)) {
            Ok(reader) => Box::new(reader),
            Err(_) => panic!("Called unwrap on an error ResultOfReader"),
        }
    }
}

/// JPEG decoder reader backed by zune-jpeg with streaming support.
///
/// Data is accumulated from the SkStreamAdapter incrementally.
/// `read_metadata()` attempts to parse headers from available data.
/// `read_image_data()` attempts full decode from available data.
/// Both return `IncompleteInput` if not enough data is available yet.
pub struct Reader {
    /// Stream for reading more data on retry.
    stream: Option<cxx::UniquePtr<ffi::SkStreamAdapter>>,
    /// Raw JPEG data accumulated from stream so far.
    raw_data: Vec<u8>,
    /// Whether we've consumed the entire stream.
    stream_exhausted: bool,
    /// Original color space from zune-jpeg header parsing.
    orig_color_space: ZuneColorSpace,
    metadata_loaded: bool,
    width: u32,
    height: u32,
    color: JpegColor,
    alpha: JpegAlpha,
    bytes_per_pixel: u32,
    image_data: Vec<u8>,
    image_data_loaded: bool,
    /// Number of fully decoded pixel rows (may be less than height for
    /// truncated images).
    valid_rows: u32,
    last_consumed_row_count: u32,
}

impl Reader {
    /// Create a new reader without reading any data from the stream.
    /// The stream is stored for later incremental reads.
    fn new(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Result<Self, DecodingResult> {
        Ok(Reader {
            stream: Some(input),
            raw_data: Vec::new(),
            stream_exhausted: false,
            orig_color_space: ZuneColorSpace::RGB,
            metadata_loaded: false,
            width: 0,
            height: 0,
            color: JpegColor::RGB,
            alpha: JpegAlpha::Opaque,
            bytes_per_pixel: 3,
            image_data: Vec::new(),
            image_data_loaded: false,
            valid_rows: 0,
            last_consumed_row_count: 0,
        })
    }

    /// Try to read more data from the stream into raw_data.
    ///
    /// Only marks the stream as exhausted when the read returns empty data
    /// (true EOF). If bytes are received, the stream may still provide more
    /// data later (e.g. in a HaltingStream streaming scenario), so
    /// `stream_exhausted` is left as-is to allow future calls to read more.
    fn try_read_more(&mut self) {
        if self.stream_exhausted {
            return;
        }
        if let Some(ref mut stream) = self.stream {
            match read_available_from_stream(stream) {
                Ok(data) => {
                    if data.is_empty() {
                        // Empty read means the stream is at its current EOF
                        // boundary — no more data is available right now.
                        self.stream_exhausted = true;
                    } else {
                        self.raw_data.extend_from_slice(&data);
                        // Got bytes: stream is not (yet) exhausted; more data
                        // may arrive later via addNewData in a HaltingStream.
                    }
                }
                Err(()) => {
                    self.stream_exhausted = true;
                }
            }
        }
    }

    pub fn width(&self) -> u32 {
        self.width
    }

    pub fn height(&self) -> u32 {
        self.height
    }

    pub fn color(&self) -> JpegColor {
        self.color
    }

    pub fn alpha(&self) -> JpegAlpha {
        self.alpha
    }

    pub fn metadata_loaded(&self) -> bool {
        self.metadata_loaded
    }

    pub fn read_metadata(&mut self) -> DecodingResult {
        if self.metadata_loaded {
            return DecodingResult::Success;
        }

        // Read available data from stream.  If fewer than 2 bytes arrived,
        // we cannot confirm the JPEG SOI marker yet.
        self.try_read_more();

        if !is_jpeg_data(&self.raw_data) {
            if self.raw_data.len() < 2 && !self.stream_exhausted {
                return DecodingResult::IncompleteInput;
            }
            return DecodingResult::FormatError;
        }

        // Try parsing headers with available data.
        let options = zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .set_max_width(usize::MAX)
            .set_max_height(usize::MAX);

        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            ZCursor::new(self.raw_data.as_slice()),
            options,
        );

        match decoder.decode_headers() {
            Ok(()) => {}
            Err(ref e) => {
                // If stream isn't exhausted and we got a format error,
                // it might be because we don't have enough data yet.
                if !self.stream_exhausted {
                    return DecodingResult::IncompleteInput;
                }
                return map_zune_error(e);
            }
        }

        let (width, height) = match decoder.dimensions() {
            Some(dims) => dims,
            None => return DecodingResult::FormatError,
        };

        let cs = decoder.input_colorspace().unwrap_or(ZuneColorSpace::RGB);
        let (color, bytes_per_pixel) = map_color_space(cs);

        self.width = width as u32;
        self.height = height as u32;
        self.color = color;
        self.alpha = JpegAlpha::Opaque;
        self.bytes_per_pixel = bytes_per_pixel;
        self.orig_color_space = cs;
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

        self.try_read_more();

        // Configure output color space to match what we reported in metadata.
        let target_cs = match self.orig_color_space {
            ZuneColorSpace::Luma | ZuneColorSpace::LumaA => ZuneColorSpace::Luma,
            _ => ZuneColorSpace::RGB,
        };

        let options = zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .set_max_width(usize::MAX)
            .set_max_height(usize::MAX)
            .jpeg_set_out_colorspace(target_cs);

        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            ZCursor::new(self.raw_data.as_slice()),
            options,
        );

        let total_bytes = (self.width * self.height * self.bytes_per_pixel) as usize;
        self.image_data = vec![0u8; total_bytes];

        match decoder.decode_into(&mut self.image_data) {
            Ok(()) => {
                self.valid_rows = self.height;
                self.image_data_loaded = true;
                DecodingResult::Success
            }
            Err(ref e) => {
                // A decode error on a non-exhausted stream means more data
                // may arrive; return IncompleteInput so the caller can retry.
                if !self.stream_exhausted {
                    self.image_data.clear();
                    return DecodingResult::IncompleteInput;
                }
                // Stream is exhausted but decode failed — estimate how many
                // rows were decoded by finding the last non-zero row in the buffer.
                let rows = self.estimate_decoded_rows();
                if rows > 0 {
                    self.valid_rows = rows;
                    self.image_data_loaded = true;
                    return DecodingResult::Success;
                }
                self.image_data.clear();
                map_zune_error(e)
            }
        }
    }

    pub fn image_data_loaded(&self) -> bool {
        self.image_data_loaded
    }

    pub fn get_next_rows<'a>(&'a mut self, buffer: &mut &'a [u8]) -> DecodedRowsInfo {
        let already_consumed = self.last_consumed_row_count;
        let row_bytes = self.row_bytes() as usize;

        // Report only the rows that were actually decoded into the buffer;
        // valid_rows may be less than height for truncated images.
        let current_rows = if self.image_data_loaded {
            self.valid_rows
        } else {
            0
        };

        if current_rows <= already_consumed {
            *buffer = &[];
            return DecodedRowsInfo {
                dst_row_start: 0,
                row_count: 0,
            };
        }

        let new_row_count = current_rows - already_consumed;
        self.last_consumed_row_count = current_rows;

        let buf_start = (already_consumed as usize) * row_bytes;
        let new_bytes = (new_row_count as usize) * row_bytes;
        *buffer = &self.image_data[buf_start..buf_start + new_bytes];

        DecodedRowsInfo {
            dst_row_start: already_consumed,
            row_count: new_row_count,
        }
    }

    pub fn row_bytes(&self) -> u32 {
        self.width * self.bytes_per_pixel
    }

    /// Estimate the number of decoded rows by scanning for the last non-zero row
    /// in the image buffer. Used for truncated image handling when the decoder
    /// partially fills the buffer in non-strict mode.
    ///
    /// Limitation: images with genuine all-zero (solid-black) trailing rows
    /// will be undercounted. This is an acceptable trade-off for truncated-
    /// image graceful degradation.
    fn estimate_decoded_rows(&self) -> u32 {
        if self.image_data.is_empty() || self.width == 0 || self.bytes_per_pixel == 0 {
            return 0;
        }
        let row_bytes = (self.width * self.bytes_per_pixel) as usize;
        for row in (0..self.height as usize).rev() {
            let start = row * row_bytes;
            let end = start + row_bytes;
            if end <= self.image_data.len() && self.image_data[start..end].iter().any(|&b| b != 0) {
                return (row + 1) as u32;
            }
        }
        0
    }

    pub fn reset_decode_state(&mut self) {
        self.image_data.clear();
        self.image_data_loaded = false;
        self.valid_rows = 0;
        self.last_consumed_row_count = 0;
        // Allow try_read_more to pick up bytes that arrived after the last failure.
        self.stream_exhausted = false;
    }

    pub fn icc_profile(&mut self) -> Vec<u8> {
        let decoder = self.make_header_decoder();
        match decoder {
            Some(d) => d.icc_profile().unwrap_or_default(),
            None => Vec::new(),
        }
    }

    pub fn exif_data(&mut self) -> Vec<u8> {
        let decoder = self.make_header_decoder();
        match decoder {
            Some(d) => d.exif().cloned().unwrap_or_default(),
            None => Vec::new(),
        }
    }

    /// Create a temporary decoder with headers parsed, for metadata extraction.
    fn make_header_decoder(&self) -> Option<zune_jpeg::JpegDecoder<ZCursor<&[u8]>>> {
        let options = zune_core::options::DecoderOptions::default()
            .set_strict_mode(false)
            .set_max_width(usize::MAX)
            .set_max_height(usize::MAX);
        let mut decoder = zune_jpeg::JpegDecoder::new_with_options(
            ZCursor::new(self.raw_data.as_slice()),
            options,
        );
        if decoder.decode_headers().is_err() {
            return None;
        }
        Some(decoder)
    }
}

pub fn new_reader(input: cxx::UniquePtr<ffi::SkStreamAdapter>) -> Box<ResultOfReader> {
    let result = Reader::new(input);
    Box::new(ResultOfReader { result })
}

// ---------------------------------------------------------------------------
// JPEG Encoder (using jpeg-encoder crate)
// ---------------------------------------------------------------------------

pub fn encode_jpeg(
    pixels: &[u8],
    width: u32,
    height: u32,
    row_bytes: u32,
    color_type: JpegEncodeColor,
    alpha_option: JpegEncodeAlpha,
    quality: u32,
    output: &mut Vec<u8>,
) -> EncodingResult {
    let src_bpp: usize = match color_type {
        JpegEncodeColor::RGB => 3,
        JpegEncodeColor::RGBA | JpegEncodeColor::BGRA => 4,
        JpegEncodeColor::Grayscale => 1,
        _ => return EncodingResult::ParameterError,
    };

    let row_bytes = row_bytes as usize;
    let w = width as usize;
    let h = height as usize;
    let min_row = w * src_bpp;

    if row_bytes < min_row || pixels.len() < h * row_bytes {
        return EncodingResult::ParameterError;
    }

    let (w16, h16) = match (u16::try_from(width), u16::try_from(height)) {
        (Ok(w), Ok(h)) => (w, h),
        _ => return EncodingResult::ParameterError,
    };

    // Convert RGBA/BGRA → RGB, handling alpha blending if requested.
    let (rgb_data, jpeg_color) = match color_type {
        JpegEncodeColor::Grayscale => {
            // Extract rows respecting row_bytes stride.
            let mut data = Vec::with_capacity(w * h);
            for y in 0..h {
                let row_start = y * row_bytes;
                data.extend_from_slice(&pixels[row_start..row_start + w]);
            }
            (data, jpeg_encoder::ColorType::Luma)
        }
        JpegEncodeColor::RGB => {
            let mut data = Vec::with_capacity(w * h * 3);
            for y in 0..h {
                let row_start = y * row_bytes;
                data.extend_from_slice(&pixels[row_start..row_start + w * 3]);
            }
            (data, jpeg_encoder::ColorType::Rgb)
        }
        JpegEncodeColor::RGBA => {
            let blend_on_black = matches!(alpha_option, JpegEncodeAlpha::BlendOnBlack);
            let mut data = Vec::with_capacity(w * h * 3);
            for y in 0..h {
                let row_start = y * row_bytes;
                for x in 0..w {
                    let px = row_start + x * 4;
                    let (r, g, b, a) = (pixels[px], pixels[px + 1], pixels[px + 2], pixels[px + 3]);
                    if blend_on_black && a < 255 {
                        let af = a as u32;
                        data.push(((r as u32 * af + 127) / 255) as u8);
                        data.push(((g as u32 * af + 127) / 255) as u8);
                        data.push(((b as u32 * af + 127) / 255) as u8);
                    } else {
                        data.push(r);
                        data.push(g);
                        data.push(b);
                    }
                }
            }
            (data, jpeg_encoder::ColorType::Rgb)
        }
        JpegEncodeColor::BGRA => {
            let blend_on_black = matches!(alpha_option, JpegEncodeAlpha::BlendOnBlack);
            let mut data = Vec::with_capacity(w * h * 3);
            for y in 0..h {
                let row_start = y * row_bytes;
                for x in 0..w {
                    let px = row_start + x * 4;
                    let (b, g, r, a) = (pixels[px], pixels[px + 1], pixels[px + 2], pixels[px + 3]);
                    if blend_on_black && a < 255 {
                        let af = a as u32;
                        data.push(((r as u32 * af + 127) / 255) as u8);
                        data.push(((g as u32 * af + 127) / 255) as u8);
                        data.push(((b as u32 * af + 127) / 255) as u8);
                    } else {
                        data.push(r);
                        data.push(g);
                        data.push(b);
                    }
                }
            }
            (data, jpeg_encoder::ColorType::Rgb)
        }
        _ => return EncodingResult::ParameterError,
    };

    let quality = quality.min(100).max(1) as u8;

    let mut buf: Vec<u8> = Vec::new();
    let encoder = jpeg_encoder::Encoder::new(&mut buf, quality);

    match encoder.encode(&rgb_data, w16, h16, jpeg_color) {
        Ok(()) => {
            *output = buf;
            EncodingResult::Success
        }
        Err(_) => EncodingResult::OtherError,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_jpeg_data() {
        assert!(is_jpeg_data(&[0xFF, 0xD8, 0xFF, 0xE0]));
        assert!(!is_jpeg_data(&[0x89, b'P', b'N', b'G']));
        assert!(!is_jpeg_data(&[b'B', b'M']));
        assert!(!is_jpeg_data(&[0xFF]));
        assert!(!is_jpeg_data(&[]));
    }

    #[test]
    fn test_map_color_space() {
        use zune_core::colorspace::ColorSpace as CS;

        let (color, bpp) = map_color_space(CS::Luma);
        assert_eq!(color, JpegColor::Grayscale);
        assert_eq!(bpp, 1);

        let (color, bpp) = map_color_space(CS::LumaA);
        assert_eq!(color, JpegColor::Grayscale);
        assert_eq!(bpp, 1);

        // Any non-luma color space should map to RGB.
        for cs in [CS::RGB, CS::RGBA, CS::BGR, CS::YCbCr] {
            let (color, bpp) = map_color_space(cs);
            assert_eq!(color, JpegColor::RGB, "expected RGB for {:?}", cs);
            assert_eq!(bpp, 3, "expected 3 bpp for {:?}", cs);
        }
    }

    // Verify that a minimal valid JPEG can be encoded and that the output
    // starts with the JPEG SOI marker.
    #[test]
    fn test_encode_jpeg_rgb_roundtrip() {
        // Solid red 2x2 RGB image.
        let pixels: Vec<u8> = vec![255, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            /*width=*/ 2,
            /*height=*/ 2,
            /*row_bytes=*/ 6,
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            /*quality=*/ 90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::Success);
        assert!(is_jpeg_data(&output), "encoded output should be a JPEG");
    }

    // Verify that invalid parameters (row_bytes too small) are rejected.
    #[test]
    fn test_encode_jpeg_rejects_invalid_row_bytes() {
        let pixels = vec![0u8; 12];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            2,
            2,
            /*row_bytes=*/ 1, // too small for width=2, bpp=3
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::ParameterError);
    }

    // Verify that oversize dimensions (> u16::MAX) are rejected.
    #[test]
    fn test_encode_jpeg_rejects_oversize_dimensions() {
        let pixels = vec![0u8; 3];
        let mut output = Vec::new();
        let result = encode_jpeg(
            &pixels,
            /*width=*/ u32::MAX,
            /*height=*/ 1,
            u32::MAX,
            JpegEncodeColor::RGB,
            JpegEncodeAlpha::Ignore,
            90,
            &mut output,
        );
        assert_eq!(result, EncodingResult::ParameterError);
    }
}
