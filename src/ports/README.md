# Skia Platform Ports (`src/ports/`)

## Overview

`src/ports/` contains Skia's platform abstraction and OS integration layer, isolating operating system and third-party font/system library dependencies from [`../core/`](../core/) and backing the factory headers in [`../../include/ports/`](../../include/ports/):

* **Font Discovery (`SkFontMgr`)**: Enumerates and matches system or custom font families across macOS/iOS (CoreText), Windows (DirectWrite/GDI), Linux (`fontconfig`), Android (`fonts.xml` and NDK), and Fuchsia.
* **Typefaces & Glyph Scalers (`SkTypeface`, `SkScalerContext`)**: Extracts glyph metrics, bitmaps, color tables, and vector outlines using platform font engines, FreeType, or the memory-safe Rust Fontations backend.
* **OS Primitives**: Implements file I/O and memory mapping (`SkOSFile`), heap allocation (`SkMemory`), debug logging (`SkLog`), and default `SkFlattenable` registration.
* **Platform Image Generators**: Bridges OS-native image codecs (CoreGraphics ImageIO, Windows Imaging Component, and Android NDK `AImageDecoder`).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`fontations/`](fontations/) | Rust `cxx` FFI bridge (`skrifa`, `read-fonts`, `font-types`) powering the memory-safe `SkTypeface_Fontations` backend. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `src/ports/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **macOS & iOS Fonts (CoreText)** | [`SkFontMgr_mac_ct.cpp`](SkFontMgr_mac_ct.cpp), [`SkTypeface_mac_ct.h`](SkTypeface_mac_ct.h), [`SkScalerContext_mac_ct.h`](SkScalerContext_mac_ct.h) | CoreText font collection discovery (`CTFontCollection`), `CTFontRef` typeface wrapping, and CoreGraphics glyph rasterization. |
| **Windows Fonts (DirectWrite & GDI)** | [`SkFontMgr_win_dw.cpp`](SkFontMgr_win_dw.cpp), [`SkTypeface_win_dw.h`](SkTypeface_win_dw.h), [`SkScalerContext_win_dw.h`](SkScalerContext_win_dw.h), [`SkFontHost_win.cpp`](SkFontHost_win.cpp) | DirectWrite font discovery, `IDWriteFontFace` metrics and ClearType/color glyph rendering, plus legacy Win32 GDI font support. |
| **FreeType & Fontations Backends** | [`SkFontHost_FreeType.cpp`](SkFontHost_FreeType.cpp), [`SkFontHost_FreeType_common.h`](SkFontHost_FreeType_common.h), [`SkTypeface_FreeType.h`](SkTypeface_FreeType.h), [`SkTypeface_fontations_priv.h`](SkTypeface_fontations_priv.h), [`SkFontScanner_fontations_priv.h`](SkFontScanner_fontations_priv.h), [`SkTypeface_proxy.h`](SkTypeface_proxy.h) | FreeType and Rust Fontations typeface/scaler implementations and shared `SkTypeface_proxy` metadata wrapper. |
| **Linux, Android, Fuchsia & Custom FontMgrs** | [`SkFontMgr_fontconfig.cpp`](SkFontMgr_fontconfig.cpp), [`SkFontMgr_FontConfigInterface.cpp`](SkFontMgr_FontConfigInterface.cpp), [`SkFontMgr_android.cpp`](SkFontMgr_android.cpp), [`SkFontMgr_android_ndk.cpp`](SkFontMgr_android_ndk.cpp), [`SkFontMgr_fuchsia.cpp`](SkFontMgr_fuchsia.cpp), [`SkFontMgr_custom.h`](SkFontMgr_custom.h) | Platform font managers for Linux `libfontconfig`/IPC, Android `fonts.xml` and NDK `ASystemFontIterator`, Fuchsia, and custom directory/embedded fonts. |
| **OS File, Memory & Logging** | [`SkOSFile_stdio.cpp`](SkOSFile_stdio.cpp), [`SkOSFile_posix.cpp`](SkOSFile_posix.cpp), [`SkOSFile_win.cpp`](SkOSFile_win.cpp), [`SkMemory_malloc.cpp`](SkMemory_malloc.cpp), [`SkLog_stdio.cpp`](SkLog_stdio.cpp), [`SkLog_android.cpp`](SkLog_android.cpp), [`SkLog_win.cpp`](SkLog_win.cpp), [`SkGlobalInitialization_default.cpp`](SkGlobalInitialization_default.cpp) | Cross-platform file I/O (`sk_fopen`, `sk_fmunmap`), heap allocation (`sk_malloc_throw`), `SkDebugf` logging, and global deserialization registration. |
| **Platform Image Codecs** | [`SkImageGeneratorCG.cpp`](SkImageGeneratorCG.cpp), [`SkImageGeneratorWIC.cpp`](SkImageGeneratorWIC.cpp), [`SkImageGeneratorNDK.cpp`](SkImageGeneratorNDK.cpp), [`SkImageEncoder_NDK.cpp`](SkImageEncoder_NDK.cpp) | OS-backed `SkImageGenerator` and encoder implementations for Apple ImageIO, Windows WIC, and Android NDK. |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../../include/ports/`](../../include/ports/) | Public factory headers for platform font managers, typefaces, and image generators. |
| [`../core/`](../core/) | Abstract `SkFontMgr`, `SkTypeface`, `SkScalerContext`, and OS primitive interfaces implemented by `src/ports/`. |
| [`../sfnt/`](../sfnt/) | Low-level TrueType/OpenType (`sfnt`) binary font table definitions shared across font ports. |
