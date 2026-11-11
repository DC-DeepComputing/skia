# Skia Public API Headers (`include/`)

## Overview

`include/` houses Skia's public C++ API headers.
These files define the supported contract between Skia and external embedders (such as Chromium, Android, and Flutter).

* **Public Subdirectories**: Exported to clients via `SK_API`.
  Public headers enforce strict Include-What-You-Use (IWYU) and Bazel `layering_check` rules, and must never `#include` headers from `src/`.
* **[`private/`](private/)**: Internal headers that must be transitively included by public headers or shared with restricted friend clients (`private/chromium/`, `private/gpu/`).
  **Symbols in `include/private/` are not part of Skia's public API or ABI** and may change or be removed without notice.

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`core/`](core/) | Fundamental 2D graphics primitives: canvas, surfaces, images, paths, paint, fonts, and color spaces. |
| [`effects/`](effects/) | Public factories for shaders (gradients, noise, runtime effects), color filters, mask filters, path effects, and image filters. |
| [`gpu/`](gpu/) | Public GPU contexts, recorders, and backend texture/semaphore interop (`graphite/`, `ganesh/`, `vk/`, `mtl/`). |
| [`codec/`](codec/) | Image decoding APIs and format-specific decoder headers. |
| [`encode/`](encode/) | Image encoding APIs (PNG, JPEG, WebP) and ICC profile utilities. |
| [`pathops/`](pathops/) | Boolean path operations (`Op`, `Simplify`, `AsWinding`, `SkOpBuilder`). |
| [`ports/`](ports/) | Platform-specific font manager, typeface, and image generator factories. |
| [`sksl/`](sksl/) | Public SkSL version and debug trace interfaces. |
| [`svg/`](svg/) | SVG canvas for recording draw commands into SVG XML streams. |
| [`utils/`](utils/) | Utility canvas subclasses, shadow helpers, 3D camera, and SVG path parsing. |
| [`docs/`](docs/) | Document generation APIs (PDF, XPS, multi-picture documents). |
| [`config/`](config/) | Compile-time configuration template (`SkUserConfig.h`). |
| [`cpu/`](cpu/) | CPU recorder context interface. |
| [`android/`](android/) | Android framework and NDK `AHardwareBuffer` interop headers. |
| [`private/`](private/) | Non-public foundational templates, containers, and friend-only APIs. |
| [`third_party/`](third_party/) | Third-party headers exposed for backend interop (such as Vulkan memory allocator headers). |

## Key Classes and Entrypoints

The primary entrypoints in [`core/`](core/) and [`effects/`](effects/) are grouped below by concept:

| Concept | Key Headers | Description |
| :--- | :--- | :--- |
| **Drawing & Recording** | [`SkCanvas.h`](core/SkCanvas.h), [`SkPicture.h`](core/SkPicture.h), [`SkPictureRecorder.h`](core/SkPictureRecorder.h), [`SkDocument.h`](core/SkDocument.h) | Immediate-mode drawing (`SkCanvas`), display-list recording and playback (`SkPicture`), and multi-page document generation. |
| **Surfaces & Images** | [`SkSurface.h`](core/SkSurface.h), [`SkImage.h`](core/SkImage.h), [`SkBitmap.h`](core/SkBitmap.h), [`SkPixmap.h`](core/SkPixmap.h), [`SkImageInfo.h`](core/SkImageInfo.h) | Render targets (`SkSurface`), immutable pixel/texture abstractions (`SkImage`), mutable bitmaps (`SkBitmap`), and transient pixel views (`SkPixmap`). |
| **Geometry & Paths** | [`SkPoint.h`](core/SkPoint.h), [`SkRect.h`](core/SkRect.h), [`SkRRect.h`](core/SkRRect.h), [`SkMatrix.h`](core/SkMatrix.h), [`SkM44.h`](core/SkM44.h), [`SkPath.h`](core/SkPath.h), [`SkPathBuilder.h`](core/SkPathBuilder.h) | 2D/3D points, rectangles, rounded rects, 3x3 and 4x4 transforms, immutable vector paths (`SkPath`), and mutable path construction (`SkPathBuilder`). |
| **Paint & Effects** | [`SkPaint.h`](core/SkPaint.h), [`SkShader.h`](core/SkShader.h), [`SkBlender.h`](core/SkBlender.h), [`SkColorFilter.h`](core/SkColorFilter.h), [`SkMaskFilter.h`](core/SkMaskFilter.h), [`SkPathEffect.h`](core/SkPathEffect.h), [`SkImageFilter.h`](core/SkImageFilter.h) | Styling state (`SkPaint`) and pluggable shading/filtering stages, with built-in effect factories in [`SkGradient.h`](effects/SkGradient.h), [`SkRuntimeEffect.h`](effects/SkRuntimeEffect.h), and [`SkImageFilters.h`](effects/SkImageFilters.h). |
| **Text & Fonts** | [`SkFont.h`](core/SkFont.h), [`SkTypeface.h`](core/SkTypeface.h), [`SkFontMgr.h`](core/SkFontMgr.h), [`SkTextBlob.h`](core/SkTextBlob.h) | Font sizing/hinting (`SkFont`), typeface handles (`SkTypeface`), platform font discovery (`SkFontMgr`), and positioned glyph runs (`SkTextBlob`). |
| **Color & Types** | [`SkColor.h`](core/SkColor.h), [`SkColorSpace.h`](core/SkColorSpace.h), [`SkColorType.h`](core/SkColorType.h), [`SkAlphaType.h`](core/SkAlphaType.h), [`SkRefCnt.h`](core/SkRefCnt.h), [`SkData.h`](core/SkData.h), [`SkStream.h`](core/SkStream.h) | Color representations, color space management, pixel layouts, reference counting (`sk_sp<T>`), byte buffers, and streams. |
