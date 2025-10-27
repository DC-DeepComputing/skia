// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! FFI Bridge for BMP Decoder
//!
//! This module provides a thin C++/Rust FFI interface layer for the BMP decoder.
//! It contains ONLY the FFI wrapper code - all actual decoding logic is in separate modules.
//!
//! # Architecture
//!
//! - **FFI.rs** (this file): Thin wrapper exposing Rust decoder to C++
//! - **bmp_constants.rs**: Shared constants and magic numbers
//! - **bmp_decoder.rs**: Core BMP decoding orchestration
//! - **bmp_header.rs**: BMP header parsing and validation
//! - **bmp_types.rs**: BMP-specific data structures and error types
//! - **bmp_jpeg_decoder.rs**: JPEG decoding using zune-jpeg
//! - **bmp_png_decoder.rs**: PNG decoding using png crate
//! - **bmp_icc.rs**: ICC color profile handling using moxcms (pure Rust)
//!
//! # Embedded Format Handling
//!
//! BMP files can contain embedded JPEG (BI_JPEG) or PNG (BI_PNG) data.
//! These are decoded using pure Rust implementations (zune-jpeg, png crate)
//! to maintain Rule of 2 compliance.
//!
//! **Benefits:**
//! - ✅ No C++ dependencies for embedded formats
//! - ✅ Rule of 2 compliant (Rust parses BMP header + Rust decodes embedded data)
//! - ✅ Production-ready with well-tested crates
//! - ✅ Simple FFI interface - no callback complexity
//!
//! **Future Enhancement:**
//! When Rust implementations of SkJpegCodec/SkPngCodec become available,
//! we can add optional processor delegation while keeping the Rust fallback.
//!
//! # FFI Interface
//!
//! The C++ interface provides:
//! - `create_decoder()`: Create a new decoder instance
//! - `decode_info()`: Parse header and get image dimensions/format
//! - `decode_pixels()`: Decode the actual pixel data (including embedded JPEG/PNG)
//! - `is_bmp_data()`: Fast BMP format detection
//!
//! All implementation details are delegated to the backend modules.

// Import the actual implementation modules
mod bmp_constants;
mod bmp_decoder;
mod bmp_header;
mod bmp_icc;
mod bmp_jpeg_decoder;
mod bmp_png_decoder;
mod bmp_types;

// Use the implementations
use bmp_decoder::{BmpDecoderImpl, EmbeddedFormat};
use bmp_header::is_bmp_data;
use bmp_types::BmpError;

#[cxx::bridge(namespace = "rust_bmp")]
mod ffi {
    /// FFI-friendly result for BMP decoding operations.
    #[derive(Debug, Clone, Copy)]
    enum DecodingResult {
        Success,
        FormatError,
        ParameterError,
        UnsupportedFeature,
        IncompleteInput,
        MemoryError,
        OtherError,
        DelegateToJpegCodec,
        DelegateToPngCodec,
    }

    #[derive(Debug, Clone, Copy)]
    enum ColorType {
        Unknown = 0,
        Grayscale = 1,
        Rgb = 2,
        Rgba = 3,
        Bgr = 4,
    }

    #[derive(Debug, Clone, Copy)]
    enum AlphaType {
        Unknown = 0,
        Opaque = 1,
        Premul = 2,
        Unpremul = 3,
    }

    /// Information about a BMP image.
    #[derive(Clone)]
    struct ImageInfo {
        width: u32,
        height: u32,
        color_type: ColorType,
        alpha_type: AlphaType,
    }

    /// Result of decoding BMP header information.
    #[derive(Clone)]
    struct DecodeInfoResult {
        success: bool,
        width: u32,
        height: u32,
        color_type: ColorType,
        alpha_type: AlphaType,
    }

    /// Embedded format type in BMP
    #[derive(Debug, Clone, Copy)]
    enum EmbeddedFormatType {
        None = 0,
        Jpeg = 1,
        Png = 2,
    }

    /// Result of decoding BMP pixel data.
    #[derive(Clone)]
    struct DecodePixelResult {
        success: bool,
        error_is_incomplete: bool, // true if error is due to incomplete data
        pixels: Vec<u8>,
        width: u32,
        height: u32,
        bytes_per_pixel: u32,
        data_is_incomplete: bool, // true if decode succeeded but data was incomplete
        embedded_format: EmbeddedFormatType, // indicates if BMP contains embedded JPEG/PNG
    }

    // Rust types and functions exposed to C++
    extern "Rust" {
        /// Main BMP decoder type
        type BmpDecoder;

        /// Create a new BMP decoder instance
        fn create_decoder() -> Box<BmpDecoder>;

        /// Decode BMP header information only
        fn decode_info(self: &mut BmpDecoder, data: &[u8]) -> DecodeInfoResult;

        /// Decode BMP pixel data
        /// Embedded JPEG/PNG formats are decoded using pure Rust implementations
        /// (zune-jpeg, png crate) and returned as fully decoded RGBA pixels.
        fn decode_pixels(self: &mut BmpDecoder, data: &[u8]) -> DecodePixelResult;

        /// Check if data appears to be a valid BMP
        fn is_bmp_data(data: &[u8]) -> bool;
    }
}

// Re-export the ffi types for internal use
pub use ffi::*;

/// BMP Decoder FFI Wrapper
///
/// This is a thin wrapper around `BmpDecoderImpl` that provides the FFI interface
/// to C++. It handles the translation between Rust types and FFI-safe types.
///
/// # Responsibilities
///
/// - Wrap the internal `BmpDecoderImpl`
/// - Convert Rust `Result` types to FFI-safe structs
/// - Map errors to appropriate error codes
/// - Maintain minimal state (just the inner decoder)
///
/// All actual decoding logic is delegated to `BmpDecoderImpl` in `decoder.rs`.
pub struct BmpDecoder {
    inner: BmpDecoderImpl,
}

/// Create a new BMP decoder instance
///
/// # Returns
///
/// A boxed `BmpDecoder` ready for use from C++
pub fn create_decoder() -> Box<BmpDecoder> {
    Box::new(BmpDecoder {
        inner: BmpDecoderImpl::new(),
    })
}

impl BmpDecoder {
    /// Decode BMP header information only
    ///
    /// Parses the BMP header to extract image metadata without decoding pixels.
    ///
    /// # Arguments
    ///
    /// * `data` - Raw BMP file data (at least header must be complete)
    ///
    /// # Returns
    ///
    /// `DecodeInfoResult` with image dimensions and format, or failure indication
    pub fn decode_info(&mut self, data: &[u8]) -> DecodeInfoResult {
        match self.inner.decode_info_ffi(data) {
            Ok((width, height, color_type, alpha_type)) => DecodeInfoResult {
                success: true,
                width,
                height,
                color_type,
                alpha_type,
            },
            Err(_) => {
                // Header parsing failed - return empty result
                DecodeInfoResult {
                    success: false,
                    width: 0,
                    height: 0,
                    color_type: ColorType::Unknown,
                    alpha_type: AlphaType::Unknown,
                }
            }
        }
    }

    /// Decode BMP pixel data
    ///
    /// Decodes the BMP pixel data into RGBA format.
    /// Embedded JPEG/PNG formats are decoded using pure Rust implementations
    /// (zune-jpeg, png crate) for Rule of 2 compliance.
    ///
    /// # Arguments
    ///
    /// * `data` - Complete BMP file data
    ///
    /// # Returns
    ///
    /// `DecodePixelResult` with decoded pixels.
    /// The `embedded_format` field indicates if the data was originally embedded JPEG/PNG.
    /// The `error_is_incomplete` field indicates if more data is needed.
    pub fn decode_pixels(&mut self, data: &[u8]) -> DecodePixelResult {
        match self.inner.decode_pixels(data) {
            Ok(decoded) => {
                // Map embedded format to FFI type
                let embedded_format = match decoded.embedded_format {
                    Some(EmbeddedFormat::Jpeg) => EmbeddedFormatType::Jpeg,
                    Some(EmbeddedFormat::Png) => EmbeddedFormatType::Png,
                    None => EmbeddedFormatType::None,
                };

                DecodePixelResult {
                    success: true,
                    error_is_incomplete: false,
                    pixels: decoded.pixels,
                    width: self.inner.width,
                    height: self.inner.height,
                    bytes_per_pixel: decoded.bytes_per_pixel,
                    data_is_incomplete: decoded.data_is_incomplete,
                    embedded_format,
                }
            }
            Err(e) => {
                // Check if error is due to incomplete data
                let is_incomplete = matches!(e, BmpError::InsufficientData);

                DecodePixelResult {
                    success: false,
                    error_is_incomplete: is_incomplete,
                    pixels: Vec::new(),
                    width: 0,
                    height: 0,
                    bytes_per_pixel: 0,
                    data_is_incomplete: false,
                    embedded_format: EmbeddedFormatType::None,
                }
            }
        }
    }
}
