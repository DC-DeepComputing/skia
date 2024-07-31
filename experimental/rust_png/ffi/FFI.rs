// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `png` Rust crate.
//! The public API of this crate is the C++ API exposed via `#[cxx::bridge]`.

// TODO(https://crbug.com/40278281): Once the FFI API matures we should move the
// `experimental/rust_png/ffi` directory to `third_party/rust/png/v0_x/ffi`.
// This directory is temporarily in Skia to avoid having to coordinate changes
// across Skia and Chromium repos.

use std::io::Read;
use std::pin::Pin;

#[cxx::bridge(namespace = "rust_png")]
mod ffi {
    /// This exposes an equivalent of `png::ColorType` to C++.
    enum ColorType {
        Grayscale = 0,
        Rgb = 2,
        Indexed = 3,
        GrayscaleAlpha = 4,
        Rgba = 6,
    }

    /// Result of decoding a PNG image.
    struct DecodedImage {
        width: u32,
        height: u32,
        color: ColorType,
        bits_per_component: u8,
        data: Vec<u8>,
    }

    unsafe extern "C++" {
        include!("third_party/skia/experimental/rust_png/ffi/FFI.h");
        type ReadTrait;

        fn read(self: Pin<&mut ReadTrait>, buffer: &mut [u8]) -> usize;
    }

    extern "Rust" {
        fn read_png(input: Pin<&mut ReadTrait>) -> UniquePtr<DecodedImage>;
    }
}

impl From<png::ColorType> for ffi::ColorType {
    fn from(value: png::ColorType) -> Self {
        match value {
            png::ColorType::Grayscale => Self::Grayscale,
            png::ColorType::Rgb => Self::Rgb,
            png::ColorType::Indexed => Self::Indexed,
            png::ColorType::GrayscaleAlpha => Self::GrayscaleAlpha,
            png::ColorType::Rgba => Self::Rgba,
        }
    }
}

impl Read for Pin<&mut ffi::ReadTrait> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        Ok(self.as_mut().read(buf))
    }
}

/// Internal helper that `read_png` is build on top of.
fn read_png_and_return_result(
    input: Pin<&mut ffi::ReadTrait>,
) -> Result<ffi::DecodedImage, png::DecodingError> {
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
fn read_png(input: Pin<&mut ffi::ReadTrait>) -> cxx::UniquePtr<ffi::DecodedImage> {
    match read_png_and_return_result(input) {
        Ok(image) => cxx::UniquePtr::new(image),
        Err(_) => cxx::UniquePtr::null(),
    }
}
