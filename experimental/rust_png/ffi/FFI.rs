// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `png` Rust crate.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

// TODO(https://crbug.com/356927699): If Rust `png` needs to be used from C++
// clients other than Skia, then we should move the FFI glue to
// `third_party/rust/png/v0_x/ffi`.

use std::io::Read;
use std::pin::Pin;

// No `use png::...` nor `use ffi::...` because we want the code to explicitly
// spell out if it means `ffi::ColorType` vs `png::ColorType` (or `Reader`
// vs `png::Reader`).

#[cxx::bridge(namespace = "rust_png")]
mod ffi {
    /// FFI-friendly equivalent of `png::ColorType`.
    enum ColorType {
        Grayscale = 0,
        Rgb = 2,
        Indexed = 3,
        GrayscaleAlpha = 4,
        Rgba = 6,
    }

    /// FFI-friendly simplification of `Option<png::DecodingError>`.
    enum DecodingResult {
        Success,
        FormatError,
        ParameterError,
        LimitsExceededError,
        // `ReadTrait` is infallible and therefore we expect no `png::DecodingError::IoError`
        // and provide no equivalent of this variant.

        // TODO(https://crbug.com/356923435): Add `IncompleteInput`.
    }

    unsafe extern "C++" {
        include!("experimental/rust_png/ffi/FFI.h");
        type ReadTrait;
        fn read(self: Pin<&mut ReadTrait>, buffer: &mut [u8]) -> usize;

        include!("third_party/skia/include/codec/SkCodec.h");
    }

    extern "Rust" {
        fn new_reader(input: UniquePtr<ReadTrait>) -> Box<ResultOfReader>;

        type ResultOfReader;
        fn err(self: &ResultOfReader) -> DecodingResult;
        fn unwrap(self: &mut ResultOfReader) -> Box<Reader>;

        type Reader;
        fn height(self: &Reader) -> u32;
        fn width(self: &Reader) -> u32;
        fn output_buffer_size(self: &Reader) -> usize;
        fn output_color_type(self: &Reader) -> ColorType;
        fn output_bits_per_component(self: &Reader) -> u8;
        fn next_frame(self: &mut Reader, output: &mut [u8]) -> DecodingResult;
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

impl From<Option<&png::DecodingError>> for ffi::DecodingResult {
    fn from(option: Option<&png::DecodingError>) -> Self {
        match option {
            None => Self::Success,
            Some(decoding_error) => match decoding_error {
                png::DecodingError::IoError(_) => {
                    // `ReadTrait` is infallible => we expect no `png::DecodingError::IoError`.
                    unreachable!()
                }
                png::DecodingError::Format(_) => Self::FormatError,
                png::DecodingError::Parameter(_) => Self::ParameterError,
                png::DecodingError::LimitsExceeded => Self::LimitsExceededError,
            },
        }
    }
}

impl Read for Pin<&mut ffi::ReadTrait> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        Ok(self.as_mut().read(buf))
    }
}

/// FFI-friendly wrapper around `Result<T, E>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct ResultOfReader(Result<Reader, png::DecodingError>);

impl ResultOfReader {
    fn err(&self) -> ffi::DecodingResult {
        self.0.as_ref().err().into()
    }

    fn unwrap(&mut self) -> Box<Reader> {
        // Leaving `self` in a C++-friendly "moved-away" state.
        let mut result = Err(png::DecodingError::LimitsExceeded);
        std::mem::swap(&mut self.0, &mut result);

        Box::new(result.unwrap())
    }
}

/// FFI-friendly wrapper around `png::Reader<R>` (`cxx` can't handle arbitrary
/// generics, so we manually monomorphize here, but still expose a minimal,
/// somewhat tweaked API of the original type).
struct Reader {
    reader: png::Reader<Pin<&'static mut ffi::ReadTrait>>,

    /// `_input` is never read/used - it just maintains ownership of the
    /// `ReadTrait` so that the reader can safely retain a pinned mutable
    /// reference to `ReadTrait`.
    _input: cxx::UniquePtr<ffi::ReadTrait>,
}

impl Reader {
    fn new(mut input: cxx::UniquePtr<ffi::ReadTrait>) -> Result<Self, png::DecodingError> {
        let pinned_input: Pin<&'static mut ffi::ReadTrait> = {
            let pin_mut = input.pin_mut();

            // SAFETY: `Reader::_input` ensures that the lifetime of `ReadTrait` is the same
            // as the lifetime of `this`.  Note that the `input` is never
            // mutated except via `pinned_input`.
            let mut_ref: &mut ffi::ReadTrait = unsafe { pin_mut.get_unchecked_mut() };
            let ptr = mut_ref as *mut ffi::ReadTrait;
            let static_mut_ref: &'static mut ffi::ReadTrait = unsafe { &mut *ptr };
            Pin::static_mut(static_mut_ref)
        };

        // Note that the default `png::Limits::bytes` is 64MiB (based on
        // https://docs.rs/png/latest/png/struct.Limits.html#structfield.bytes).
        let mut decoder = png::Decoder::new(pinned_input);

        // `EXPAND` will:
        // * Expand bit depth to at least 8 bits
        // * Translate palette indices into RGB or RGBA
        //
        // TODO(https://crbug.com/356882657): Consider handling palette expansion
        // via `SkSwizzler` instead of relying on `EXPAND` for this use case.
        decoder.set_transformations(png::Transformations::EXPAND);

        let reader = decoder.read_info()?;
        Ok(Self { reader, _input: input })
    }

    fn height(&self) -> u32 {
        self.reader.info().height
    }

    fn width(&self) -> u32 {
        self.reader.info().width
    }

    fn output_buffer_size(&self) -> usize {
        self.reader.output_buffer_size()
    }

    fn output_color_type(&self) -> ffi::ColorType {
        self.reader.output_color_type().0.into()
    }

    fn output_bits_per_component(&self) -> u8 {
        self.reader.output_color_type().1 as u8
    }

    fn next_frame(&mut self, output: &mut [u8]) -> ffi::DecodingResult {
        self.reader.next_frame(output).as_ref().err().into()
    }
}

/// This provides a public C++ API for decoding a PNG image.
fn new_reader(input: cxx::UniquePtr<ffi::ReadTrait>) -> Box<ResultOfReader> {
    Box::new(ResultOfReader(Reader::new(input)))
}
