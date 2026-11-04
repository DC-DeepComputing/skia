# Skia Image Encoders (`src/encode/`)

## Overview

`src/encode/` implements Skia's image encoding framework and format-specific encoders, backing the public encoding APIs in [`../../include/encode/`](../../include/encode/):

* **Incremental & One-Shot Encoding**: Converts raster pixel buffers (`SkPixmap`, `SkBitmap`), planar `SkYUVAPixmaps`, or `SkImage` instances into compressed streams (`SkWStream`, `SkData`).
* **Format Encoders**: Provides PNG (`libpng` and Rust `png`), JPEG (`libjpeg-turbo`), and WebP (`libwebp`) encoders with configurable compression, quality, and animation options.
* **ICC Profile Serialization**: Generates standards-compliant ICC color profile blobs from `skcms` transfer functions and gamut matrices, including HDR CICP and lookup table tags.
* **HDR Gainmap Encoding**: Assembles multi-picture format (MPF) JPEG streams with ISO 21496-1 and XMP metadata for UltraHDR images.

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/encode/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Base Encoder & Row Helpers** | [`SkEncoder.cpp`](SkEncoder.cpp), [`SkImageEncoderPriv.h`](SkImageEncoderPriv.h), [`SkImageEncoderFns.h`](SkImageEncoderFns.h) | Stateful scanline encoder base (`SkEncoder::encodeRows`), input pixmap validation, and scanline transformation/EXIF orientation helpers. |
| **PNG Encoders** | [`SkPngEncoderBase.h`](SkPngEncoderBase.h), [`SkPngEncoderImpl.h`](SkPngEncoderImpl.h), [`SkPngRustEncoderImpl.h`](SkPngRustEncoderImpl.h) | Shared scanline conversion base (`SkPngEncoderBase`) and concrete `libpng` (`SkPngEncoderImpl`) and memory-safe Rust `png` (`SkPngRustEncoderImpl`) backends. |
| **JPEG Encoder** | [`SkJpegEncoderImpl.h`](SkJpegEncoderImpl.h), [`SkJPEGWriteUtility.h`](SkJPEGWriteUtility.h) | `libjpeg-turbo` encoder (`SkJpegEncoderImpl`) supporting RGB/YUVA inputs, chroma downsampling, and `SkWStream` destination bridging (`skjpeg_destination_mgr`). |
| **WebP Encoder** | [`SkWebpEncoderImpl.cpp`](SkWebpEncoderImpl.cpp) | `libwebp` encoder supporting lossy/lossless compression, ICC profile embedding via `WebPMux`, and multi-frame animated WebP encoding. |
| **ICC Profiles & HDR Gainmaps** | [`SkICC.cpp`](SkICC.cpp), [`SkICCPriv.h`](SkICCPriv.h), [`SkJpegGainmapEncoder.cpp`](SkJpegGainmapEncoder.cpp) | ICC profile synthesis (`SkWriteICCProfile`) and UltraHDR / MPF JPEG gainmap assembly (`SkJpegGainmapEncoder`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/encode/`](../../include/encode/) | Public encoder headers, format options structs, and ICC profile APIs. |
| [`../codec/`](../codec/) | Counterpart image decoding framework and shared JPEG/PNG/HDR metadata utilities. |
| [`../../rust/png/`](../../rust/png/) | Rust FFI bridge backing the Rust PNG encoder. |
| [`../../modules/skcms/`](../../modules/skcms/) | Color space representations and ICC profile data structures. |
