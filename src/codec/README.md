# Skia Image Codecs (`src/codec/`)

## Overview

`src/codec/` implements Skia's image decoding framework and format-specific decoders, backing the public decoding APIs in [`../../include/codec/`](../../include/codec/):

* **Stream & Data Decoding**: Translates encoded byte streams (`SkStream`, `SkData`) into pixel buffers (`SkPixmap`, `SkBitmap`), planar YUVA buffers (`SkYUVAPixmaps`), or deferred `SkImageGenerator`s.
* **Flexible Decoding Modes**: Supports full-frame decoding (`getPixels`), scanline-by-scanline streaming, incremental network decoding, subsampled scaling (`SkAndroidCodec`), and multi-frame animation playback.
* **Swizzling & Color Management**: Converts source bit depths, color palettes, byte orders, and alpha premultiplication into destination `SkImageInfo` formats with ICC/CICP color profile transformations.
* **Metadata, Orientation & HDR Gainmaps**: Parses EXIF orientation (`SkEncodedOrigin`), TIFF IFDs, XMP metadata, and ISO 21496-1 / UltraHDR gainmaps.

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/codec/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Core Codec & Android Scaling** | [`SkCodec.cpp`](SkCodec.cpp), [`SkCodecPriv.h`](SkCodecPriv.h), [`SkAndroidCodec.cpp`](SkAndroidCodec.cpp), [`SkSampledCodec.h`](SkSampledCodec.h), [`SkFrameHolder.h`](SkFrameHolder.h) | Format discovery and full/scanline/incremental decoding (`SkCodec`), subsampled/region decoding (`SkAndroidCodec`, `SkSampledCodec`), and multi-frame animation state (`SkFrameHolder`). |
| **Swizzling & Color Profiles** | [`SkSwizzler.h`](SkSwizzler.h), [`SkMaskSwizzler.h`](SkMaskSwizzler.h), [`SkSampler.h`](SkSampler.h), [`SkColorPalette.h`](SkColorPalette.h), [`SkCodecColorProfile.cpp`](SkCodecColorProfile.cpp), [`SkCodecColorProfileRust.h`](SkCodecColorProfileRust.h) | Row format conversion, bit-depth/palette expansion, subsampling (`SkSwizzler`, `SkSampler`), and ICC color profile parsing (`skcms` and Rust ICC). |
| **Deferred Image Generation** | [`SkCodecImageGenerator.h`](SkCodecImageGenerator.h), [`SkImageGenerator_FromEncoded.cpp`](SkImageGenerator_FromEncoded.cpp) | Bridges `SkCodec` to lazy `SkImageGenerator` instantiation for deferred image decoding. |
| **Metadata, EXIF & HDR Gainmaps** | [`SkExif.cpp`](SkExif.cpp), [`SkTiffUtility.h`](SkTiffUtility.h), [`SkParseEncodedOrigin.h`](SkParseEncodedOrigin.h), [`SkXmp.cpp`](SkXmp.cpp), [`SkGainmapInfo.cpp`](SkGainmapInfo.cpp), [`SkHdrMetadata.cpp`](SkHdrMetadata.cpp), [`SkHdrAgtmPriv.h`](SkHdrAgtmPriv.h) | EXIF/TIFF metadata and orientation parsing, XMP extraction, and HDR metadata / UltraHDR gainmap parsing. |
| **PNG & JPEG Decoders** | [`SkPngCodecBase.h`](SkPngCodecBase.h), [`SkPngCodec.h`](SkPngCodec.h), [`SkPngRustCodec.h`](SkPngRustCodec.h), [`SkJpegCodec.h`](SkJpegCodec.h), [`SkJpegDecoderMgr.h`](SkJpegDecoderMgr.h), [`SkJpegMultiPicture.h`](SkJpegMultiPicture.h) | PNG decoding via `libpng` (`SkPngCodec`) or Rust `png` (`SkPngRustCodec`, including APNG), and JPEG decoding via `libjpeg-turbo` (`SkJpegCodec`) with MPF/gainmap support. |
| **WebP, GIF, AVIF & Other Formats** | [`SkWebpCodec.h`](SkWebpCodec.h), [`SkWuffsCodec.cpp`](SkWuffsCodec.cpp), [`SkAvifCodec.h`](SkAvifCodec.h), [`SkCrabbyAvifCodec.h`](SkCrabbyAvifCodec.h), [`SkJpegxlCodec.h`](SkJpegxlCodec.h), [`SkBmpCodec.h`](SkBmpCodec.h), [`SkBmpRustCodec.h`](SkBmpRustCodec.h), [`SkIcoCodec.h`](SkIcoCodec.h), [`SkWbmpCodec.h`](SkWbmpCodec.h), [`SkRawCodec.h`](SkRawCodec.h) | Format decoders for WebP (`libwebp`), GIF (Wuffs), AVIF (`libavif` / `CrabbyAvif`), JPEG XL (`libjxl`), BMP (C++ and Rust), ICO, WBMP, and camera RAW (`dng_sdk` / `piex`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/codec/`](../../include/codec/) | Public `SkCodec`, `SkAndroidCodec`, and per-format decoder registration headers. |
| [`../encode/`](../encode/) | Counterpart image encoding framework and format encoders. |
| [`../../rust/`](../../rust/) | Rust FFI crates (`png`, `bmp`, `icc`, `exif`) backing memory-safe decoders and metadata parsers. |
| [`../../modules/skcms/`](../../modules/skcms/) | ICC profile parsing and color space transformation engine. |
