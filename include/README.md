# Skia Public API Headers (`include/`)

## Overview & Scope

`include/` houses Skia's public C++ API headers. These files define the supported contract between Skia and external embedders (such as Chromium, Android, and Flutter).

### Public API vs. `include/private/`
* **Public Subdirectories (`core/`, `effects/`, `gpu/`, `codec/`, `encode/`, etc.)**: Exported to clients via `SK_API`. Public headers enforce strict Include-What-You-Use (IWYU) and Bazel `layering_check` rules, and must never `#include` headers from `src/`.
* **`include/private/`**: Internal headers that must be transitively included by public headers (such as `SkTemplates.h`, `SkTArray.h`, `SkMacros.h`, `SkAssert.h`, or `SkPathRef.h`) or shared with restricted friend clients (`include/private/chromium/`, `include/private/gpu/`). **Symbols in `include/private/` are not part of Skia's public API or ABI** and may change or be removed without notice.

## Subdirectories & Related Modules (Directory Map)

| Directory | Architectural Role & Contents |
| :--- | :--- |
| [`core/`](core/) | Fundamental 2D graphics primitives, canvas, surfaces, images, paths, paint, fonts, and color spaces. Impl in [`../src/core/`](../src/core/README.md) and [`../src/image/`](../src/image/README.md). |
| [`effects/`](effects/) | Built-in shaders (gradients, Perlin noise), SkSL runtime effects, color filters, mask filters, path effects, and image filters. Impl in [`../src/effects/`](../src/effects/README.md) and [`../src/shaders/`](../src/shaders/README.md). |
| [`gpu/`](gpu/) | Public GPU backend contexts, recorders, and backend texture/semaphore interop (`ganesh/`, `graphite/`, `vk/`, `mtl/`). See [`../src/gpu/ganesh/README.md`](../src/gpu/ganesh/README.md) and [`../src/gpu/graphite/README.md`](../src/gpu/graphite/README.md). |
| [`codec/`](codec/) | Image decoding APIs ([`SkCodec.h`](codec/SkCodec.h), [`SkAndroidCodec.h`](codec/SkAndroidCodec.h)) and format-specific decoders (PNG, JPEG, WebP, AVIF, BMP, GIF, RAW). |
| [`encode/`](encode/) | Image encoding APIs ([`SkPngEncoder.h`](encode/SkPngEncoder.h), [`SkJpegEncoder.h`](encode/SkJpegEncoder.h), [`SkWebpEncoder.h`](encode/SkWebpEncoder.h), [`SkICC.h`](encode/SkICC.h)). |
| [`pathops/`](pathops/) | Boolean path operations (`Op`, `Simplify`, `AsWinding`, `TightBounds`) declared in [`SkPathOps.h`](pathops/SkPathOps.h). |
| [`ports/`](ports/) | Platform-specific `SkFontMgr`, `SkFontScanner`, `SkTypeface`, and `SkImageGenerator` factories (CoreText, DirectWrite, FreeType, Fontations, FontConfig, Android, Fuchsia). |
| [`sksl/`](sksl/) | Public Skia Shading Language (SkSL) version and debug trace interfaces ([`SkSLVersion.h`](sksl/SkSLVersion.h), [`SkSLDebugTrace.h`](sksl/SkSLDebugTrace.h)). |
| [`svg/`](svg/) | [`SkSVGCanvas.h`](svg/SkSVGCanvas.h) for recording `SkCanvas` draw commands into SVG XML streams. |
| [`utils/`](utils/) | Specialized canvas subclasses ([`SkNWayCanvas.h`](utils/SkNWayCanvas.h), [`SkNoDrawCanvas.h`](utils/SkNoDrawCanvas.h), [`SkPaintFilterCanvas.h`](utils/SkPaintFilterCanvas.h)), shadow helpers ([`SkShadowUtils.h`](utils/SkShadowUtils.h)), 3D camera, and SVG path parsing. |
| [`docs/`](docs/) | Document generation backends ([`SkPDFDocument.h`](docs/SkPDFDocument.h), [`SkXPSDocument.h`](docs/SkXPSDocument.h), [`SkMultiPictureDocument.h`](docs/SkMultiPictureDocument.h)). |
| [`config/`](config/) | Compile-time configuration template ([`SkUserConfig.h`](config/SkUserConfig.h)) and Bazel compiler/linker flags. |
| [`cpu/`](cpu/) | CPU recorder context interface ([`Recorder.h`](cpu/Recorder.h)). |
| [`android/`](android/) | Android framework and NDK `AHardwareBuffer` interop headers (`SkImageAndroid.h`, `SkSurfaceAndroid.h`, `SkAnimatedImage.h`). |
| [`private/`](private/) | Non-public foundational templates, containers, thread annotations, and friend-only APIs (`chromium/`, `gpu/`). |
| [`third_party/`](third_party/) | Third-party header integrations exposed for backend interop (such as Vulkan memory allocator headers). |

## Key Abstractions & Entrypoints

The table below groups the landmark headers in `include/core/` and `include/effects/` by concept:

| Concept | Landmark Headers | Description |
| :--- | :--- | :--- |
| **Drawing & Canvas** | [`SkCanvas.h`](core/SkCanvas.h), [`SkPicture.h`](core/SkPicture.h), [`SkPictureRecorder.h`](core/SkPictureRecorder.h), [`SkDrawable.h`](core/SkDrawable.h), [`SkDocument.h`](core/SkDocument.h), [`SkRecorder.h`](core/SkRecorder.h) | Immediate-mode drawing interface (`SkCanvas`), display-list recording (`SkPictureRecorder`) and immutable playback (`SkPicture`), and multi-page document generation. |
| **Surfaces & Images** | [`SkSurface.h`](core/SkSurface.h), [`SkImage.h`](core/SkImage.h), [`SkBitmap.h`](core/SkBitmap.h), [`SkPixmap.h`](core/SkPixmap.h), [`SkImageInfo.h`](core/SkImageInfo.h), [`SkYUVAPixmaps.h`](core/SkYUVAPixmaps.h) | Render targets (`SkSurface`), immutable CPU/GPU/lazy pixel abstractions (`SkImage`), mutable raster bitmaps (`SkBitmap`), and transient pixel views (`SkPixmap`). |
| **Geometry & Paths** | [`SkPoint.h`](core/SkPoint.h), [`SkRect.h`](core/SkRect.h), [`SkRRect.h`](core/SkRRect.h), [`SkMatrix.h`](core/SkMatrix.h), [`SkM44.h`](core/SkM44.h), [`SkPath.h`](core/SkPath.h), [`SkPathBuilder.h`](core/SkPathBuilder.h), [`SkRegion.h`](core/SkRegion.h) | 2D/3D points, rectangles, rounded rects, 3x3 (`SkMatrix`) and 4x4 (`SkM44`) transforms, immutable vector paths (`SkPath`), and mutable path construction (`SkPathBuilder`). |
| **Paint & Styling** | [`SkPaint.h`](core/SkPaint.h), [`SkShader.h`](core/SkShader.h), [`SkBlender.h`](core/SkBlender.h), [`SkBlendMode.h`](core/SkBlendMode.h), [`SkColorFilter.h`](core/SkColorFilter.h), [`SkMaskFilter.h`](core/SkMaskFilter.h), [`SkPathEffect.h`](core/SkPathEffect.h), [`SkImageFilter.h`](core/SkImageFilter.h) | Styling state (`SkPaint`) and pluggable pipeline stages: [`SkGradient.h`](effects/SkGradient.h), [`SkRuntimeEffect.h`](effects/SkRuntimeEffect.h), [`SkImageFilters.h`](effects/SkImageFilters.h), [`SkDashPathEffect.h`](effects/SkDashPathEffect.h), and [`SkBlurMaskFilter.h`](effects/SkBlurMaskFilter.h). |
| **Text & Fonts** | [`SkFont.h`](core/SkFont.h), [`SkTypeface.h`](core/SkTypeface.h), [`SkFontMgr.h`](core/SkFontMgr.h), [`SkTextBlob.h`](core/SkTextBlob.h), [`SkFontMetrics.h`](core/SkFontMetrics.h) | Font sizing/hinting configuration (`SkFont`), typeface handles (`SkTypeface`), platform font discovery (`SkFontMgr`), and positioned glyph runs (`SkTextBlob`). Note: Full Unicode text shaping lives in `modules/skshaper/`. |
| **Color & ColorSpaces** | [`SkColor.h`](core/SkColor.h), [`SkColorSpace.h`](core/SkColorSpace.h), [`SkColorType.h`](core/SkColorType.h), [`SkAlphaType.h`](core/SkAlphaType.h), [`SkColorMatrix.h`](effects/SkColorMatrix.h) | 8-bit (`SkColor`) and float (`SkColor4f`) colors, ICC/gamut/transfer-function management (`SkColorSpace`), pixel memory layouts (`SkColorType`), and premultiplication (`SkAlphaType`). |
| **Streams, Data & Lifecycle** | [`SkRefCnt.h`](core/SkRefCnt.h), [`SkData.h`](core/SkData.h), [`SkStream.h`](core/SkStream.h), [`SkString.h`](core/SkString.h), [`SkSpan.h`](core/SkSpan.h), [`SkFlattenable.h`](core/SkFlattenable.h) | Intrusive reference counting (`sk_sp<T>`), immutable byte buffers (`SkData`), I/O streams (`SkStream`/`SkWStream`), contiguous views (`SkSpan`), and object serialization (`SkFlattenable`). |

### GPU Backend Headers (`include/gpu/`)
Public GPU headers are split by engine architecture under [`include/gpu/`](gpu/):
* **Graphite** ([`include/gpu/graphite/`](gpu/graphite/)): Modern GPU backend (`Context`, `Recorder`, `Recording`, `BackendTexture`, `Surface`, `Image`). For architecture and implementation details, see [`../src/gpu/graphite/README.md`](../src/gpu/graphite/README.md).
* **Ganesh** ([`include/gpu/ganesh/`](gpu/ganesh/)): Legacy GPU backend (`GrDirectContext`, `GrRecordingContext`, `GrBackendSurface`, `SkSurfaceGanesh`, `SkImageGanesh`). For architecture and implementation details, see [`../src/gpu/ganesh/README.md`](../src/gpu/ganesh/README.md).
* **Shared GPU Types**: API-agnostic descriptors and Vulkan/Metal backend types live in [`GpuTypes.h`](gpu/GpuTypes.h), [`MutableTextureState.h`](gpu/MutableTextureState.h), [`include/gpu/vk/`](gpu/vk/), and [`include/gpu/mtl/`](gpu/mtl/).

## Build & Test

* **Bazel**: Each subdirectory defines a `BUILD.bazel` exporting header filegroups (such as `//include/core:core_hdrs`, `//include/effects:core_hdrs`, and `//include/private:private_hdrs`) consumed by `//src/core:core` and other modules. `generate_cpp_files_for_headers` targets verify that every header compiles standalone.
* **GN**: Exported into `gn/core.gni` (`skia_core_public`), `gn/effects.gni` (`skia_effects_public`), `gn/effects_imagefilters.gni`, and `gn/gpu.gni`. After modifying any `BUILD.bazel` file list, regenerate `.gni` files with `make -C bazel generate_gni`.
* **Tests**: Public API contracts are exercised across `tests/` and visual rendering corpus tests in `gm/`.
