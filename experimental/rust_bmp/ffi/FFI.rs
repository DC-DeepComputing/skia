// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++/Rust FFI for BMP decoder. Embedded JPEG/PNG formats are decoded using
//! pure Rust implementations (zune-jpeg, png crate) for Rule of 2 compliance.

mod bmp_constants;
mod bmp_decoder;
mod bmp_header;
mod bmp_icc;
mod bmp_jpeg_decoder;
mod bmp_png_decoder;
mod bmp_types;

use bmp_decoder::{BmpDecoderImpl, EmbeddedFormat};
pub use bmp_header::is_bmp_data;
use bmp_types::BmpError;

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

    struct DecodeInfoResult {
        success: bool,
        width: u32,
        height: u32,
        color_type: ColorType,
        alpha_type: AlphaType,
    }

    #[derive(Debug, Clone, Copy)]
    enum EmbeddedFormatType {
        None = 0,
        Jpeg = 1,
        Png = 2,
    }

    struct DecodePixelResult {
        success: bool,
        error_is_incomplete: bool,
        pixels: Vec<u8>,
        width: u32,
        height: u32,
        bytes_per_pixel: u32,
        data_is_incomplete: bool,
        embedded_format: EmbeddedFormatType,
    }

    extern "Rust" {
        type BmpDecoder;

        fn create_decoder() -> Box<BmpDecoder>;
        fn decode_info(self: &mut BmpDecoder, data: &[u8]) -> DecodeInfoResult;
        fn decode_pixels(self: &mut BmpDecoder, data: &[u8]) -> DecodePixelResult;
        fn is_bmp_data(data: &[u8]) -> bool;
    }
}

pub use ffi::*;

pub struct BmpDecoder {
    inner: BmpDecoderImpl,
}

pub fn create_decoder() -> Box<BmpDecoder> {
    Box::new(BmpDecoder {
        inner: BmpDecoderImpl::new(),
    })
}

impl BmpDecoder {
    pub fn decode_info(&mut self, data: &[u8]) -> DecodeInfoResult {
        match self.inner.decode_info_ffi(data) {
            Ok((width, height, color_type, alpha_type)) => DecodeInfoResult {
                success: true,
                width,
                height,
                color_type,
                alpha_type,
            },
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
        match self.inner.decode_pixels(data) {
            Ok(decoded) => {
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
