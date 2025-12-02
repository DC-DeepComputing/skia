# JXL Rust Decoder Bindings

This directory contains Rust-based bindings for JPEG XL image decoding using the
`jxl-rs` crate, exposed to C++ through CXX.

## Overview

The `FFI.rs` file provides a C++-compatible API for the `jxl-rs` JPEG XL decoder.
It follows the same pattern as the PNG Rust decoder in `../png/`.

## Key Features

- Full JXL decoding support (static and animated images)
- Multiple pixel formats (RGBA8, RGBA16, RGBA_F32)
- ICC color profile support
- Animation support with frame timing
- Wide gamut and HDR support via F32 output

## Usage

The decoder is used by `SkJxlRsCodec` in Skia's codec infrastructure:

```cpp
#include "rust/jxl/FFI.rs.h"

auto decoder = rust_jxl::new_decoder();
decoder->set_input(data, data_size, /*all_input=*/true);

while (true) {
  auto status = decoder->process();
  if (status == rust_jxl::DecodingStatus::BasicInfo) {
    auto info = decoder->get_basic_info();
    // Use width, height, has_alpha, etc.
  } else if (status == rust_jxl::DecodingStatus::FullImage) {
    decoder->get_pixels(buffer);
    break;
  }
}
```

## API

See `FFI.rs` for the complete API documentation.

## Build

The decoder is integrated into Skia's build system through `BUILD.bazel` and
GN build files.
