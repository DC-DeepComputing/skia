# Skia High-Level Modules (`modules/`)

## Overview

`modules/` contains self-contained libraries ranging from zero-dependency utilities used by core Skia to higher-level feature libraries layered on top of core Skia (`//:core`) that integrate external dependencies or scene-graph abstractions:

* **Color Management & Foundational Utilities**: Standalone ICC color profile parsing and pixel transformation ([`skcms/`](skcms/)), zero-copy DOM JSON parsing ([`jsonreader/`](jsonreader/)), and sweep-line polygon intersection algorithms ([`bentleyottmann/`](bentleyottmann/)).
* **Text Shaping & Paragraph Layout**: Unicode text segmentation and bidirectional (BiDi) analysis ([`skunicode/`](skunicode/)), low-level OpenType glyph shaping ([`skshaper/`](skshaper/)), and rich multi-line paragraph layout, hit-testing, and painting ([`skparagraph/`](skparagraph/)).
* **Vector Animation & Scene Graph**: Retained-mode 2D scene graph with dirty-region invalidation ([`sksg/`](sksg/)), external asset loading interfaces ([`skresources/`](skresources/)), Lottie animation playback ([`skottie/`](skottie/)), and SVG 1.1 DOM parsing and rendering ([`svg/`](svg/)).
* **Web Bindings & Demos**: WebAssembly/JavaScript bindings ([`canvaskit/`](canvaskit/)), cross-platform audio playback helpers ([`audioplayer/`](audioplayer/)), and an experimental plain-text editor widget ([`skplaintexteditor/`](skplaintexteditor/)).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`skcms/`](skcms/) | Standalone C11 ICC color profile parser and color space transform engine linked directly by core Skia. |
| [`skunicode/`](skunicode/) | Unicode text segmentation, BiDi reordering, and grapheme/word/line breaking backed by ICU, ICU4X, libgrapheme, or platform APIs. |
| [`skshaper/`](skshaper/) | Low-level text shaping converting UTF-8 runs into positioned glyphs via HarfBuzz or CoreText. |
| [`skparagraph/`](skparagraph/) | Rich multi-line paragraph layout, styling, font fallback, hit-testing, and painting. |
| [`sksg/`](sksg/) | Retained-mode 2D scene graph with dirty-region invalidation for vector animations. |
| [`skresources/`](skresources/) | Asset loading interfaces and proxies for external images, multi-frame animations, audio tracks, and fonts. |
| [`jsonreader/`](jsonreader/) | Fast, arena-backed DOM-style JSON parser. |
| [`skottie/`](skottie/) | Lottie (Bodymovin) JSON vector animation player built on [`sksg/`](sksg/), [`skresources/`](skresources/), [`jsonreader/`](jsonreader/), and [`skshaper/`](skshaper/). |
| [`svg/`](svg/) | SVG 1.1 DOM parser and renderer backed by Expat, [`skresources/`](skresources/), and [`skshaper/`](skshaper/). |
| [`canvaskit/`](canvaskit/) | Emscripten/WebAssembly and JavaScript bindings exposing core Skia, [`skottie/`](skottie/), [`skparagraph/`](skparagraph/), and [`svg/`](svg/) to the web. |
| [`bentleyottmann/`](bentleyottmann/) | Sweep-line algorithm for finding polygon segment intersections. |
| [`audioplayer/`](audioplayer/) | Cross-platform audio playback helper used by media-enabled tools and viewers. |
| [`skplaintexteditor/`](skplaintexteditor/) | Experimental plain-text editor widget and demo application. |
| [`pathops/`](pathops/) | Legacy GN build file placeholder (`pathops.gni`); the PathOps implementation resides in [`../src/pathops/`](../src/pathops/). |

## Key Classes and Entrypoints

The primary classes and entrypoints for modules without a dedicated subdirectory README are grouped below by concept (see [`skottie/`](skottie/), [`skparagraph/`](skparagraph/), [`skshaper/`](skshaper/), [`canvaskit/`](canvaskit/), and [`skplaintexteditor/`](skplaintexteditor/) for their respective entrypoints):

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Color Management (`skcms`)** | [`skcms.h`](skcms/skcms.h), [`skcms_public.h`](skcms/src/skcms_public.h) | ICC profile parsing (`skcms_ICCProfile`, `skcms_Parse`), transfer function math (`skcms_TransferFunction`), and pixel format/gamut conversion (`skcms_Transform`). |
| **Unicode Segmentation (`skunicode`)** | [`SkUnicode.h`](skunicode/include/SkUnicode.h), [`SkUnicode_icu.h`](skunicode/include/SkUnicode_icu.h), [`SkUnicode_icu4x.h`](skunicode/include/SkUnicode_icu4x.h), [`SkUnicode_libgrapheme.h`](skunicode/include/SkUnicode_libgrapheme.h) | Abstract Unicode interface (`SkUnicode`), BiDi region and break iterators (`SkBidiIterator`, `SkBreakIterator`), and backend factories (`SkUnicodes::ICU`, `SkUnicodes::ICU4X`, `SkUnicodes::Libgrapheme`). |
| **Scene Graph (`sksg`)** | [`SkSGScene.h`](sksg/include/SkSGScene.h), [`SkSGRenderNode.h`](sksg/include/SkSGRenderNode.h), [`SkSGNode.h`](sksg/include/SkSGNode.h), [`SkSGInvalidationController.h`](sksg/include/SkSGInvalidationController.h) | Retained scene root (`sksg::Scene`), renderable DAG nodes (`sksg::RenderNode`), base DAG node invalidation (`sksg::Node`), and dirty-region tracking (`sksg::InvalidationController`). |
| **External Resources (`skresources`)** | [`SkResources.h`](skresources/include/SkResources.h) | Embedder asset loading interface (`skresources::ResourceProvider`), static/multi-frame image proxies (`ImageAsset`, `MultiFrameImageAsset`), audio track control (`ExternalTrackAsset`), and file/data-URI/caching providers. |
| **JSON Parsing (`jsonreader`)** | [`SkJSONReader.h`](jsonreader/SkJSONReader.h) | Arena-allocated JSON DOM (`skjson::DOM`) and 64-bit tagged value facades (`skjson::Value`, `ObjectValue`, `ArrayValue`, `StringValue`). |
| **SVG DOM (`svg`)** | [`SkSVGDOM.h`](svg/include/SkSVGDOM.h), [`SkSVGSVG.h`](svg/include/SkSVGSVG.h), [`SkSVGNode.h`](svg/include/SkSVGNode.h), [`SkSVGRenderContext.h`](svg/include/SkSVGRenderContext.h) | SVG document builder and renderer (`SkSVGDOM`), root `<svg>` element (`SkSVGSVG`), DOM element base class (`SkSVGNode`), and cascading presentation state (`SkSVGRenderContext`). |
| **Geometry & Audio Utilities** | [`BentleyOttmann1.h`](bentleyottmann/include/BentleyOttmann1.h), [`SkAudioPlayer.h`](audioplayer/SkAudioPlayer.h) | Sweep-line segment intersection finder (`bentleyottmann::bentley_ottmann_1`) and cross-platform audio playback interface (`SkAudioPlayer`). |
