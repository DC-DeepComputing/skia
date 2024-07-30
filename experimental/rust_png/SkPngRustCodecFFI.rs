// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++/Rust bindings for:
//!
//! * The `png` Rust crate
//! * A subset of public Skia APIs (e.g. `SkStream`)
//!
//! The public API of this crate is the C++ API exposed via `#[cxx::bridge]`.

use std::io::Read;
use std::pin::Pin;

#[cxx::bridge(namespace = "ffi")]
mod ffi {
    /// FFI representation of `SkEncodedInfo::Color`.
    #[repr(i32)]
    enum SkEncodedInfoColor {
        kGray_Color,
        kGrayAlpha_Color,
        kXAlpha_Color,
        k565_Color,
        kPalette_Color,
        kRGB_Color,
        kRGBA_Color,
        kBGR_Color,
        kBGRX_Color,
        kBGRA_Color,
        kYUV_Color,
        kYUVA_Color,
        kInvertedCMYK_Color,
        kYCCK_Color,
    }

    /// Result of decoding a PNG image.
    struct DecodedImage {
        width: u32,
        height: u32,
        color: SkEncodedInfoColor,
        bits_per_component: u8,
        data: Vec<u8>,
    }

    unsafe extern "C++" {
        include!("experimental/rust_png/SkPngRustCodecFFI.h");
        type SkEncodedInfoColor;
        type SkStream;

        fn SkStreamRead(stream: Pin<&mut SkStream>, buffer: &mut [u8]) -> usize;
    }

    #[namespace = "ffi"]
    extern "Rust" {
        fn read_png(input: Pin<&mut SkStream>) -> UniquePtr<DecodedImage>;
    }
}

impl From<png::ColorType> for ffi::SkEncodedInfoColor {
    fn from(value: png::ColorType) -> Self {
        match value {
            png::ColorType::Grayscale => Self::kGray_Color,
            png::ColorType::Rgb => Self::kRGB_Color,
            png::ColorType::Indexed => Self::kPalette_Color,
            png::ColorType::GrayscaleAlpha => Self::kGrayAlpha_Color,
            png::ColorType::Rgba => Self::kRGBA_Color,
        }
    }
}

impl Read for Pin<&mut ffi::SkStream> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        Ok(ffi::SkStreamRead(self.as_mut(), buf))
    }
}

/// Internal helper that `read_png` is build on top of.
fn rusty_read_png(input: Pin<&mut ffi::SkStream>) -> Result<ffi::DecodedImage, png::DecodingError> {
    // Note that the default `png::Limits::bytes` is 64MiB (based on
    // https://docs.rs/png/latest/png/struct.Limits.html#structfield.bytes).
    let mut decoder = png::Decoder::new(input);

    // EXPAND:
    // * Expand bit depth to at least 8 bits
    // * Translate palette indices into RGB or RGBA
    //
    // TODO(https://crbug.com/40278281): Consider handling palette expansion via
    // `SkSwizzler` (hoping that less hops = faster transforms).
    decoder.set_transformations(png::Transformations::EXPAND);

    let mut reader = decoder.read_info()?;

    let mut output = vec![0; reader.output_buffer_size()];
    let info = reader.next_frame(&mut output)?;

    let color = info.color_type.into();
    let width = info.width;
    let height = info.height;
    let bits_per_component = info.bit_depth as u8;

    Ok(ffi::DecodedImage { width, height, color, bits_per_component, data: output })
}

/// Public API that C++ can use to decode a PNG image.
fn read_png(input: Pin<&mut ffi::SkStream>) -> cxx::UniquePtr<ffi::DecodedImage> {
    match rusty_read_png(input) {
        Ok(image) => cxx::UniquePtr::new(image),
        Err(_) => cxx::UniquePtr::null(),
    }
}
