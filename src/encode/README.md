# Skia Image Encoders (`src/encode/`)

## Overview & Scope
`src/encode/` contains Skia's image encoding framework and format-specific encoders for PNG, JPEG, and WebP, along with ICC color profile generation and HDR Gainmap encoding utilities. It converts in-memory pixel buffers (`SkPixmap`, `SkBitmap`), YUVA planes (`SkYUVAPixmaps`), or `SkImage` instances (both raster and GPU-backed) into compressed byte streams (`SkWStream`, `SkData`).

Public headers reside in [`include/encode/`](../../include/encode/), including [`SkEncoder.h`](../../include/encode/SkEncoder.h), [`SkPngEncoder.h`](../../include/encode/SkPngEncoder.h), [`SkPngRustEncoder.h`](../../include/encode/SkPngRustEncoder.h), [`SkJpegEncoder.h`](../../include/encode/SkJpegEncoder.h), [`SkWebpEncoder.h`](../../include/encode/SkWebpEncoder.h), and [`SkICC.h`](../../include/encode/SkICC.h).

## Key Abstractions & Entrypoints

### Base Encoder & Pixel Transformation Pipeline
- **`SkEncoder` ([`SkEncoder.cpp`](SkEncoder.cpp), [`include/encode/SkEncoder.h`](../../include/encode/SkEncoder.h))**: The stateful base class for scanline-by-scanline image encoding.
  - Clients create an encoder via format-specific `Make(SkWStream*, const SkPixmap&, const Options&)` factories and drive encoding incrementally with `encodeRows(int numRows)` (which delegates to `onEncodeRows`), or use one-shot `Encode(...)` helpers that return a `bool` or `sk_sp<SkData>` (including `GrDirectContext*` overloads that read back GPU textures before encoding).
- **Row Transformation Helpers ([`SkImageEncoderFns.h`](SkImageEncoderFns.h), [`SkImageEncoderPriv.h`](SkImageEncoderPriv.h))**: Inline scanline transform functions (`transform_scanline_*`) using `skcms_Transform` to convert source `SkColorType`s (such as `kBGRA_8888`, `kRGB_565`, `kRGBA_F16`, `kRGBA_1010102`) and premultiplied alpha into the unpremultiplied RGB/RGBA/Grayscale layouts expected by underlying encoder libraries.

### Format Encoders
- **PNG (`libpng` & Rust `png`)**:
  - [`SkPngEncoderBase.h`](SkPngEncoderBase.h) / [`SkPngEncoderBase.cpp`](SkPngEncoderBase.cpp): Shared base class (`SkPngEncoderBase`) that determines target `TargetInfo` pixel formats and drives per-scanline conversion from `SkPixmap` before passing rows to the concrete PNG backend.
  - **`SkPngEncoder` ([`SkPngEncoderImpl.h`](SkPngEncoderImpl.h), [`SkPngEncoderImpl.cpp`](SkPngEncoderImpl.cpp), [`include/encode/SkPngEncoder.h`](../../include/encode/SkPngEncoder.h))**: `libpng`-backed encoder supporting configurable zlib compression levels (`fZLibLevel`), filter flags (`FilterFlag`), color profiles, CICP HDR chunks, and custom comment chunks (`fComments`). Stubbed by [`SkPngEncoder_none.cpp`](SkPngEncoder_none.cpp) when disabled.
  - **`SkPngRustEncoder` ([`SkPngRustEncoder.cpp`](SkPngRustEncoder.cpp), [`SkPngRustEncoderImpl.h`](SkPngRustEncoderImpl.h), [`SkPngRustEncoderImpl.cpp`](SkPngRustEncoderImpl.cpp), [`include/encode/SkPngRustEncoder.h`](../../include/encode/SkPngRustEncoder.h))**: Memory-safe Rust `png` crate implementation (bridged via [`rust/png/`](../../rust/png/)) offering `CompressionLevel` options (`kLow`, `kMedium`, `kHigh`) and ICC/CICP metadata embedding.
- **JPEG (`libjpeg-turbo`)**:
  - **`SkJpegEncoder` ([`SkJpegEncoderImpl.h`](SkJpegEncoderImpl.h), [`SkJpegEncoderImpl.cpp`](SkJpegEncoderImpl.cpp), [`include/encode/SkJpegEncoder.h`](../../include/encode/SkJpegEncoder.h))**: `libjpeg-turbo`-backed encoder supporting quality (`0..100`), chroma downsampling (`Downsample::k420`, `k422`, `k444`), alpha handling (`AlphaOption::kIgnore`, `kBlendOnBlack`), XMP metadata, EXIF/origin segments, and direct `SkYUVAPixmaps` encoding.
  - [`SkJPEGWriteUtility.h`](SkJPEGWriteUtility.h) / [`SkJPEGWriteUtility.cpp`](SkJPEGWriteUtility.cpp): Implements `skjpeg_destination_mgr` to bridge `libjpeg` output buffers to `SkWStream`. Stubbed by [`SkJpegEncoder_none.cpp`](SkJpegEncoder_none.cpp) when disabled.
- **WebP (`libwebp`)**:
  - **`SkWebpEncoder` ([`SkWebpEncoderImpl.cpp`](SkWebpEncoderImpl.cpp), [`include/encode/SkWebpEncoder.h`](../../include/encode/SkWebpEncoder.h))**: `libwebp`-backed encoder supporting lossy (`Compression::kLossy`) and lossless (`Compression::kLossless`) compression, quality/effort controls, ICC profile embedding via `WebPMux`, and multi-frame animated WebP encoding (`SkSpan<const SkEncoder::Frame>`). Stubbed by [`SkWebpEncoder_none.cpp`](SkWebpEncoder_none.cpp) when disabled.

### ICC Profiles & HDR Gainmap Encoding
- **ICC Profile Generation ([`SkICC.cpp`](SkICC.cpp), [`SkICCPriv.h`](SkICCPriv.h), [`include/encode/SkICC.h`](../../include/encode/SkICC.h))**: Implements `SkWriteICCProfile`, serializing `skcms_TransferFunction` and chromaticity gamut matrices (`toXYZD50`) into standards-compliant ICC profile `SkData` blobs, including CICP (`cicp` tag) and A2B0 lookup tables for PQ/HLG HDR transfer functions. Also used by the PDF backend (`src/pdf/`).
- **JPEG Gainmap / UltraHDR Encoding ([`SkJpegGainmapEncoder.cpp`](SkJpegGainmapEncoder.cpp))**: Implements `SkJpegGainmapEncoder::EncodeHDRGM` and `MakeMPF`, assembling multi-picture format (MPF) JPEG streams with ISO 21496-1 and Adobe XMP `hdrgm` metadata for UltraHDR images.

## Subdirectories & Related Modules
- [`../../include/encode/`](../../include/encode/): Public encoder headers and options structs.
- [`../codec/`](../codec/): Shares common PNG/JPEG constants (`SkPngPriv.h`, `SkJpegConstants.h`), HDR metadata (`:hdr_metadata`), and MPF segment utilities.
- [`../../rust/png/`](../../rust/png/): Rust FFI bridge backing `SkPngRustEncoder`.
- [`../../modules/skcms/`](../../modules/skcms/): Color space conversion routines and ICC profile data structures.

## Build & Test
- **Bazel (`BUILD.bazel`)**: Defines `:encoder_common`, `:icc_support` / `:icc_encode`, `:png_encode_base`, `:png_encode`, `:rust_png_encoder`, `:jpeg_encode`, and `:webp_encode`.
- **GN (`../../gn/core.gni` & `../../gn/skia.gni`)**: Controlled by `skia_use_libpng_encode`, `skia_use_rust_png_encode`, `skia_use_libjpeg_turbo_encode`, `skia_use_jpeg_gainmaps`, and `skia_use_libwebp_encode`.
- **Tests & GMs**: Covered by `tests/EncodeTest.cpp`, `tests/SkPngRustEncoderTest.cpp`, `tests/ICCTest.cpp`, `tests/JpegGainmapTest.cpp`, `tests/NdkEncodeTest.cpp`, and `gm/encode*.cpp`.
