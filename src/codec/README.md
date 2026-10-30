# Skia Image Codecs (`src/codec/`)

## Overview & Scope
`src/codec/` contains Skia's image decoding framework and format-specific decoders. It translates encoded image streams/data (`SkStream`, `SkData`) into raw pixel buffers (`SkPixmap`, `SkBitmap`), YUVA planes (`SkYUVAPixmaps`), or lazy `SkImageGenerator`s, while handling color space profiles, EXIF orientation, HDR gainmaps, scaling, and multi-frame animations.

Public headers reside in [`include/codec/`](../../include/codec/), centered on [`SkCodec.h`](../../include/codec/SkCodec.h), [`SkAndroidCodec.h`](../../include/codec/SkAndroidCodec.h), and per-format decoder registration headers (`SkPngDecoder.h`, `SkJpegDecoder.h`, etc.).

## Key Abstractions & Entrypoints

### Core Decoding Architecture & Registration
- **`SkCodec` ([`SkCodec.cpp`](SkCodec.cpp), [`include/codec/SkCodec.h`](../../include/codec/SkCodec.h))**: The primary abstraction over an encoded image stream.
  - **Codec Registration & Discovery**: Clients either pass an explicit `SkSpan<const SkCodecs::Decoder>` to `SkCodec::MakeFromStream` / `SkCodec::MakeFromData` or register decoders globally via `SkCodecs::Register(...)` (using format `Decoder()` factories from `include/codec/Sk*Decoder.h`). `SkCodec` peeks the stream header (up to `MinBufferedBytesNeeded()`) to match the format.
  - **Full, Scanline &Incremental Decoding**: `getPixels()` decodes a full frame; `startScanlineDecode()` / `getScanlines()` / `skipScanlines()` support row-by-row streaming; `startIncrementalDecode()` / `incrementalDecode()` support progressive/networked streams.
  - **Animated / Multi-Frame Images**: `getFrameCount()`, `getFrameInfo()`, and `Options::fFrameIndex` / `fPriorFrame` (backed by [`SkFrameHolder.h`](SkFrameHolder.h) and [`include/codec/SkCodecAnimation.h`](../../include/codec/SkCodecAnimation.h)) handle frame disposal, blending, and dependency chains in GIF, WebP, APNG, and AVIF.
- **`SkAndroidCodec` ([`SkAndroidCodec.cpp`](SkAndroidCodec.cpp), [`include/codec/SkAndroidCodec.h`](../../include/codec/SkAndroidCodec.h))**: Wraps `SkCodec` (via [`SkAndroidCodecAdapter.h`](SkAndroidCodecAdapter.h) or [`SkSampledCodec.h`](SkSampledCodec.h)) to provide simplified subsampled scaling (`getSampledDimensions`) and subset/region decoding (`AndroidOptions::fSubset`).
- **Swizzling & Color Conversion**: [`SkSwizzler.cpp`](SkSwizzler.cpp) ([`SkSwizzler.h`](SkSwizzler.h)), [`SkMaskSwizzler.cpp`](SkMaskSwizzler.cpp), and [`SkSampler.cpp`](SkSampler.cpp) convert source bit depths, color palettes ([`SkColorPalette.h`](SkColorPalette.h)), byte orders, alpha premultiplication, and horizontal subsampling into the target `SkImageInfo`, integrating with `skcms` via [`SkCodecColorProfile.cpp`](SkCodecColorProfile.cpp) (or Rust ICC via [`SkCodecColorProfileRust.cpp`](SkCodecColorProfileRust.cpp)).
- **Lazy Image Generation**: [`SkCodecImageGenerator.cpp`](SkCodecImageGenerator.cpp) and [`SkImageGenerator_FromEncoded.cpp`](SkImageGenerator_FromEncoded.cpp) bridge `SkCodec` to `SkImage::DeferredFromEncodedData`.

### Metadata, Orientation & HDR / Gainmaps
- **EXIF, TIFF & Orientation**: [`SkExif.cpp`](SkExif.cpp), [`SkTiffUtility.cpp`](SkTiffUtility.cpp), [`SkParseEncodedOrigin.cpp`](SkParseEncodedOrigin.cpp), and [`SkPixmapUtils.cpp`](SkPixmapUtils.cpp) parse TIFF IFDs, EXIF metadata, and `SkEncodedOrigin` transforms.
- **HDR, Gainmaps & XMP**: [`SkGainmapInfo.cpp`](SkGainmapInfo.cpp), [`SkHdrMetadata.cpp`](SkHdrMetadata.cpp), [`SkHdrAgtm.cpp`](SkHdrAgtm.cpp) / [`SkHdrAgtmParse.cpp`](SkHdrAgtmParse.cpp) (SMPTE ST 2094-50 / AGTM), [`SkXmp.cpp`](SkXmp.cpp), [`SkJpegXmp.cpp`](SkJpegXmp.cpp), [`SkJpegMultiPicture.cpp`](SkJpegMultiPicture.cpp), and [`SkJpegSegmentScan.cpp`](SkJpegSegmentScan.cpp) extract UltraHDR / ISO 21496-1 gainmap images and HDR metadata (`onGetGainmapCodec`, `onGetGainmapInfo`).

### Format Decoders
- **PNG (`libpng` & Rust `png`)**: Shared base [`SkPngCodecBase.cpp`](SkPngCodecBase.cpp) and chunk reader [`SkPngCompositeChunkReader.cpp`](SkPngCompositeChunkReader.cpp) support both C-based [`SkPngCodec.cpp`](SkPngCodec.cpp) (`SkPngDecoder` via `libpng`) and memory-safe Rust-based [`SkPngRustCodec.cpp`](SkPngRustCodec.cpp) / [`SkPngRustDecoder.cpp`](SkPngRustDecoder.cpp) (`SkPngRustDecoder`, including APNG support).
- **JPEG (`libjpeg-turbo`)**: [`SkJpegCodec.cpp`](SkJpegCodec.cpp), [`SkJpegDecoderMgr.cpp`](SkJpegDecoderMgr.cpp), [`SkJpegSourceMgr.cpp`](SkJpegSourceMgr.cpp), [`SkJpegUtility.cpp`](SkJpegUtility.cpp), and [`SkJpegMetadataDecoderImpl.cpp`](SkJpegMetadataDecoderImpl.cpp) implement `SkJpegDecoder`, supporting native IDCT downscaling and YUVA decoding.
- **WebP (`libwebp`)**: [`SkWebpCodec.cpp`](SkWebpCodec.cpp) (`SkWebpDecoder`) supports lossy/lossless WebP, arbitrary scaling, subsets, and animation.
- **GIF (`wuffs`)**: [`SkWuffsCodec.cpp`](SkWuffsCodec.cpp) (`SkGifDecoder`) uses memory-safe Wuffs for incremental and animated GIF decoding.
- **AVIF (`libavif` & `CrabbyAvif`)**: [`SkAvifCodec.cpp`](SkAvifCodec.cpp) and Android's Rust-backed [`SkCrabbyAvifCodec.cpp`](SkCrabbyAvifCodec.cpp) (`SkAvifDecoder`).
- **JPEG XL (`libjxl`)**: [`SkJpegxlCodec.cpp`](SkJpegxlCodec.cpp) (`SkJpegxlDecoder`).
- **BMP, WBMP & ICO**: [`SkBmpCodec.cpp`](SkBmpCodec.cpp) (with standard, RLE, and bitmask subclasses) and Rust-backed [`SkBmpRustCodec.cpp`](SkBmpRustCodec.cpp) (`SkBmpDecoder` / `SkBmpRustDecoder`); [`SkWbmpCodec.cpp`](SkWbmpCodec.cpp) (`SkWbmpDecoder`); [`SkIcoCodec.cpp`](SkIcoCodec.cpp) (`SkIcoDecoder`, delegating embedded entries to BMP or PNG codecs).
- **RAW (`dng_sdk` & `piex`)**: [`SkRawCodec.cpp`](SkRawCodec.cpp) (`SkRawDecoder`) decodes camera RAW/DNG files or extracts preview JPEGs via PIEX.

## Subdirectories & Related Modules
- [`../../include/codec/`](../../include/codec/): Public `SkCodec`, `SkAndroidCodec`, and format `Sk*Decoder.h` headers.
- [`../../rust/png/`](../../rust/png/), [`../../rust/bmp/`](../../rust/bmp/), [`../../rust/icc/`](../../rust/icc/), [`../../rust/exif/`](../../rust/exif/), and [`../../experimental/rust_jpeg/`](../../experimental/rust_jpeg/): Rust FFI crates backing the memory-safe codec implementations.
- [`../../modules/skcms/`](../../modules/skcms/): ICC color profile parsing and pixel color space transformations.
- [`../encode/`](../encode/): Counterpart image encoding implementations.

## Build & Test
- **Bazel (`BUILD.bazel`)**: Defines `:any_decoder` (core `SkCodec` infrastructure), `:hdr_metadata`, `:android_codec`, and modular codec targets (`:png_decode`, `:rust_png_decoder`, `:jpeg_decode`, `:webp_decode`, `:gif_decode`, `:bmp_decode`, `:rust_bmp_decoder`, `:ico_decode`, `:wbmp_decode`, `:jpegxl_decode`, `:rust_icc_support`).
- **GN (`../../gn/codec.gni` & `../../gn/skia.gni`)**: Controlled by `skia_use_libpng_decode`, `skia_use_rust_png_decode`, `skia_use_libjpeg_turbo_decode`, `skia_use_jpeg_gainmaps`, `skia_use_libwebp_decode`, `skia_use_wuffs`, `skia_use_libavif`, `skia_use_crabbyavif`, `skia_use_libjxl_decode`, `skia_use_dng_sdk`, `skia_use_rust_bmp_decode`, and `skia_use_rust_icc`.
- **Tests**: Covered by `tests/CodecTest.cpp`, `tests/CodecAnimTest.cpp`, `tests/CodecPartialTest.cpp`, `tests/AndroidCodecTest.cpp`, `tests/JpegGainmapTest.cpp`, `tests/PngGainmapTest.cpp`, `tests/ExifTest.cpp`, `tests/AvifTest.cpp`, and `tests/BadIcoTest.cpp`.
