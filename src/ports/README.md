# Skia Platform Ports (`src/ports/`)

## Overview & Scope
`src/ports/` contains Skia's platform abstraction and OS integration layer. Because Skia's core (`src/core/`) is strictly platform-independent, all operating system and third-party font/system library dependencies are isolated here.

Its primary responsibilities are:
- **Font Management & Font Rendering**: Platform-specific implementations of `SkFontMgr`, `SkTypeface`, `SkScalerContext`, and `SkFontScanner`, paired with public factory headers in [`include/ports/`](../../include/ports/).
- **OS Primitives**: File I/O (`SkOSFile`), heap allocation (`SkMemory`), debug logging (`SkLog`), discardable memory, and global initialization (`SkGlobalInitialization`).
- **Platform Image Codecs**: Native OS image decoding/encoding bridges (`SkImageGenerator` and NDK encoders).

## Key Abstractions & Entrypoints

### Font Managers (`SkFontMgr`) & Typeface / Scaler Contexts (`SkTypeface` / `SkScalerContext`)
When a client requests a font via `SkFontMgr` (using factories declared in [`include/ports/`](../../include/ports/)), the platform port resolves system font configuration and instantiates a backend-specific `SkTypeface`. When text is measured or rasterized at a specific size/matrix, `SkTypeface::onCreateScalerContext` creates an `SkScalerContext` to generate glyph metrics, bitmaps, and paths:

- **macOS & iOS (CoreText)**:
  - [`SkFontMgr_mac_ct.cpp`](SkFontMgr_mac_ct.cpp) ([`include/ports/SkFontMgr_mac_ct.h`](../../include/ports/SkFontMgr_mac_ct.h)) enumerates and matches `CTFontCollection` / `CTFontDescriptor` fonts.
  - [`SkTypeface_mac_ct.cpp`](SkTypeface_mac_ct.cpp) and [`SkScalerContext_mac_ct.cpp`](SkScalerContext_mac_ct.cpp) wrap `CTFontRef` and CoreGraphics glyph rasterization/outlines.
- **Windows (DirectWrite & GDI)**:
  - [`SkFontMgr_win_dw.cpp`](SkFontMgr_win_dw.cpp), [`SkTypeface_win_dw.cpp`](SkTypeface_win_dw.cpp), and [`SkScalerContext_win_dw.cpp`](SkScalerContext_win_dw.cpp) ([`include/ports/SkTypeface_win.h`](../../include/ports/SkTypeface_win.h)) implement DirectWrite font discovery, `IDWriteFontFace` metrics, ClearType/grayscale rendering, and color glyph tables.
  - [`SkFontHost_win.cpp`](SkFontHost_win.cpp) provides legacy Win32 GDI font support.
- **Linux, Android & Embedded (FreeType, FontConfig, Config Parsers)**:
  - [`SkFontHost_FreeType.cpp`](SkFontHost_FreeType.cpp) and [`SkFontHost_FreeType_common.cpp`](SkFontHost_FreeType_common.cpp) implement `SkTypeface_FreeType` ([`SkTypeface_FreeType.h`](SkTypeface_FreeType.h)), `SkScalerContext_FreeType`, and `SkFontScanner_FreeType` ([`include/ports/SkFontScanner_FreeType.h`](../../include/ports/SkFontScanner_FreeType.h)).
  - **Linux**: [`SkFontMgr_fontconfig.cpp`](SkFontMgr_fontconfig.cpp) ([`include/ports/SkFontMgr_fontconfig.h`](../../include/ports/SkFontMgr_fontconfig.h)) queries `libfontconfig`, while [`SkFontMgr_FontConfigInterface.cpp`](SkFontMgr_FontConfigInterface.cpp) and [`SkFontConfigInterface_direct.cpp`](SkFontConfigInterface_direct.cpp) support sandboxed IPC font access (e.g., Chromium).
  - **Android**: [`SkFontMgr_android.cpp`](SkFontMgr_android.cpp) and [`SkFontMgr_android_parser.cpp`](SkFontMgr_android_parser.cpp) parse `fonts.xml` system configurations; [`SkFontMgr_android_ndk.cpp`](SkFontMgr_android_ndk.cpp) uses Android's NDK `ASystemFontIterator` API.
  - **Fuchsia & Custom**: [`SkFontMgr_fuchsia.cpp`](SkFontMgr_fuchsia.cpp) integrates with `fuchsia.fonts`; [`SkFontMgr_custom.cpp`](SkFontMgr_custom.cpp) backs directory ([`SkFontMgr_custom_directory.cpp`](SkFontMgr_custom_directory.cpp)), embedded data ([`SkFontMgr_custom_embedded.cpp`](SkFontMgr_custom_embedded.cpp)), and empty ([`SkFontMgr_custom_empty.cpp`](SkFontMgr_custom_empty.cpp)) font managers.
- **Fontations (Rust)**:
  - [`SkTypeface_fontations.cpp`](SkTypeface_fontations.cpp), [`SkFontScanner_fontations.cpp`](SkFontScanner_fontations.cpp), and [`SkFontMgr_fontations_empty.cpp`](SkFontMgr_fontations_empty.cpp) ([`include/ports/SkTypeface_fontations.h`](../../include/ports/SkTypeface_fontations.h)) implement a memory-safe Rust font backend via the `cxx` bridge in [`fontations/`](fontations/).
- **Typeface Proxy**: [`SkTypeface_proxy.cpp`](SkTypeface_proxy.cpp) ([`SkTypeface_proxy.h`](SkTypeface_proxy.h)) allows `SkFontMgr` implementations to wrap a real `SkTypeface` while overriding style or family metadata.

### OS Primitives & Platform Image Generators
- **File System (`SkOSFile`)**: [`SkOSFile_stdio.cpp`](SkOSFile_stdio.cpp), [`SkOSFile_posix.cpp`](SkOSFile_posix.cpp), [`SkOSFile_win.cpp`](SkOSFile_win.cpp), and [`SkOSFile_ios.h`](SkOSFile_ios.h) implement `sk_fopen`, memory mapping (`sk_fmunmap`), and directory iteration.
- **Memory & Logging**: [`SkMemory_malloc.cpp`](SkMemory_malloc.cpp) and [`SkMemory_mozalloc.cpp`](SkMemory_mozalloc.cpp) implement `sk_malloc_throw`/`sk_free`; [`SkLog_stdio.cpp`](SkLog_stdio.cpp), [`SkLog_android.cpp`](SkLog_android.cpp), and [`SkLog_win.cpp`](SkLog_win.cpp) route `SkDebugf` to `stderr`, `logcat`, or `OutputDebugString`. [`SkDiscardableMemory_none.cpp`](SkDiscardableMemory_none.cpp) stubs discardable memory when disabled.
- **Global Init**: [`SkGlobalInitialization_default.cpp`](SkGlobalInitialization_default.cpp) registers default `SkFlattenable` subclasses for deserialization.
- **Platform Image Generators**: [`SkImageGeneratorCG.cpp`](SkImageGeneratorCG.cpp) (macOS/iOS ImageIO), [`SkImageGeneratorWIC.cpp`](SkImageGeneratorWIC.cpp) (Windows Imaging Component), and [`SkImageGeneratorNDK.cpp`](SkImageGeneratorNDK.cpp) / [`SkImageEncoder_NDK.cpp`](SkImageEncoder_NDK.cpp) (Android `AImageDecoder` / `AndroidBitmap`).

## Subdirectories & Related Modules
- [`fontations/`](fontations/): Rust `cxx` FFI bridge (`skrifa`, `read-fonts`, `font-types`) powering `SkTypeface_fontations`.
- [`../../include/ports/`](../../include/ports/): Public factory headers for platform font managers, typefaces, and image generators.
- [`../core/`](../core/): Defines the abstract `SkFontMgr`, `SkTypeface`, `SkScalerContext`, and `SkOSFile` interfaces implemented here.
- [`../sfnt/`](../sfnt/): Low-level OpenType/TrueType table structs shared by several font ports.

## Build & Test
- **Bazel (`BUILD.bazel`)**: Defines modular targets including `:core_srcs` (`:osfile`, `:malloc`, `:skdebug`, `:global_init`), `:freetype_support`, `:fontations_support`, `:fontmgr_coretext`, `:fontmgr_win_dw`, `:fontmgr_fontconfig`, `:fontmgr_android`, `:fontmgr_android_ndk`, and `:imagegenerator`.
- **GN (`../../gn/ports.gni` & `../../gn/skia.gni`)**: Controlled by feature flags such as `skia_use_freetype`, `skia_use_fontations`, `skia_use_fontconfig`, `skia_use_fonthost_mac`, `skia_enable_fontmgr_win`, `skia_enable_fontmgr_android`, and `skia_use_ndk_images`.
- **Tests**: Covered by `tests/FontMgrTest.cpp`, `tests/FontHostTest.cpp`, `tests/FontationsTest.cpp`, `tests/FontationsFtCompTest.cpp`, `tests/FontScanner*Test.cpp`, `tests/FontMgrAndroidParserTest.cpp`, `tests/FontMgrFontConfigTest.cpp`, and `tests/OSPathTest.cpp`.
