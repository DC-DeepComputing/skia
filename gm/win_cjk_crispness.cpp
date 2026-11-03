/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// DO NOT SUBMIT: experiment for https://github.com/flutter/flutter/issues/194931
// Compares 14px CJK text across Light Mode and Dark Mode in Skia vs native GDI/DirectWrite

#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkBase64.h"
#include "src/core/SkUTF.h"
#include "tools/fonts/FontToolUtils.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <vector>

#if defined(SK_BUILD_FOR_WIN)
#include "include/ports/SkTypeface_win.h"
#include "src/core/SkLeanWindows.h"
#include "src/utils/win/SkTScopedComPtr.h"
#if defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
#include <dwrite.h>
#include <dwrite_1.h>
#include "src/ports/SkTypeface_win_dw.h"
#endif
#endif

namespace {

constexpr int kLabelWidth = 280;
constexpr int kColWidth = 530;
constexpr int kCols = 3;
constexpr int kWidth = kLabelWidth + kCols * kColWidth;
constexpr int kHeaderHeight = 24;
constexpr int kRowHeight = 46;
constexpr int kRows = 20;
constexpr int kHeight = kHeaderHeight + kRows * kRowHeight + 10;

constexpr const char* kLine1 = "你好，世界 Hello World 0123456789";
constexpr const char* kLine2 =
        "在 RenderObject 树和 Layer 树的协作下，只有标记为 dirty 的节点才会参与重绘。";

struct ThemeCol {
    const char* fTitle;
    uint8_t fFgR;
    uint8_t fFgG;
    uint8_t fFgB;
    uint8_t fBgR;
    uint8_t fBgG;
    uint8_t fBgB;

    SkColor fgSkColor() const { return SkColorSetRGB(fFgR, fFgG, fFgB); }
    SkColor bgSkColor() const { return SkColorSetRGB(fBgR, fBgG, fBgB); }
    uint32_t bgDibPixel() const {
        return (static_cast<uint32_t>(fBgR) << 16) | (static_cast<uint32_t>(fBgG) << 8) |
               static_cast<uint32_t>(fBgB);
    }
#if defined(SK_BUILD_FOR_WIN)
    COLORREF fgColorRef() const { return RGB(fFgR, fFgG, fFgB); }
    COLORREF bgColorRef() const { return RGB(fBgR, fBgG, fBgB); }
#endif
};

// Col 1 matches Impeller's kLightTone -> SK_ColorWHITE pre-blend path; Col 2 matches M3 dark UI
constexpr ThemeCol kThemes[kCols] = {
        {"Col 0: Light (#000000 on #FFFFFF)", 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF},
        {"Col 1: Dark (#FFFFFF on #000000, Impeller kLightTone)",
         0xFF,
         0xFF,
         0xFF,
         0x00,
         0x00,
         0x00},
        {"Col 2: Dark UI (#E6E1E5 on #1E1E1E, M3 dark)", 0xE6, 0xE1, 0xE5, 0x1E, 0x1E, 0x1E},
};

sk_sp<SkTypeface> match_cjk(const sk_sp<SkFontMgr>& mgr, SkString* family) {
    if (!mgr) {
        return nullptr;
    }
    for (const char* name : {"Microsoft YaHei", "Microsoft YaHei UI", "SimSun", "Noto Sans SC"}) {
        if (sk_sp<SkTypeface> tf = mgr->matchFamilyStyle(name, SkFontStyle())) {
            *family = name;
            return tf;
        }
    }
    return nullptr;
}

SkString fallback_family_name(const sk_sp<SkFontMgr>& mgr,
                              const char* primaryFam,
                              const char* bcp47,
                              int bcp47Count,
                              SkUnichar cp) {
    if (!mgr) {
        return SkString("none");
    }
    const char** locales = bcp47 ? &bcp47 : nullptr;
    sk_sp<SkTypeface> tf =
            mgr->matchFamilyStyleCharacter(primaryFam, SkFontStyle(), locales, bcp47Count, cp);
    if (!tf) {
        return SkString("null");
    }
    SkString fam;
    tf->getFamilyName(&fam);
    return fam;
}

enum class RowKind {
    kSkia,
    kSkiaFallbackEnUs,
    kNativeGdiClearType,
    kNativeGdiAntialiased,
    kNativeDwClearTypeDefault,
    kNativeDwClearTypeGdiClassic,
    kNativeDwGrayscaleDefault,
    kNativeDwGrayscaleNatSym,
};

struct Variant {
    const char* fName;
    RowKind fKind;
    bool fGdi;
    SkFont::Edging fEdging;
    SkFontHinting fHinting;
    bool fSubpixel;
    bool fUseSysDwGamma;
    SkScalar fGamma;
    SkScalar fContrast;
};

constexpr SkFont::Edging kA8 = SkFont::Edging::kAntiAlias;
constexpr SkFont::Edging kLCD = SkFont::Edging::kSubpixelAntiAlias;
constexpr SkScalar kDefG = SK_GAMMA_EXPONENT;
constexpr SkScalar kDefC = SK_GAMMA_CONTRAST;

// Compares current Skia sRGB pre-blend, native Win32 GDI/DW targets, gamma/contrast sweeps,
// and Segoe UI + en-US CJK fallback across Light and Dark themes
constexpr Variant kVariants[kRows] = {
        {"00 Skia DW A8 (sRGB c=0.5) [cur]",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         kDefG,
         kDefC},
        {"01 Native GDI ExtTextOut CLEARTYPE",
         RowKind::kNativeGdiClearType,
         true,
         kLCD,
         SkFontHinting::kNormal,
         false,
         false,
         kDefG,
         kDefC},
        {"02 Native GDI ExtTextOut ANTIALIASED",
         RowKind::kNativeGdiAntialiased,
         true,
         kA8,
         SkFontHinting::kNormal,
         false,
         false,
         kDefG,
         kDefC},
        {"03 Native DW BitmapRT CT DEFAULT",
         RowKind::kNativeDwClearTypeDefault,
         false,
         kLCD,
         SkFontHinting::kNormal,
         true,
         false,
         kDefG,
         kDefC},
        {"04 Native DW BitmapRT CT GDI_CLASSIC",
         RowKind::kNativeDwClearTypeGdiClassic,
         false,
         kLCD,
         SkFontHinting::kFull,
         false,
         false,
         kDefG,
         kDefC},
        {"05 Native DW BitmapRT Gray DEFAULT",
         RowKind::kNativeDwGrayscaleDefault,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         kDefG,
         kDefC},
        {"06 Native DW BitmapRT Gray NAT_SYM",
         RowKind::kNativeDwGrayscaleNatSym,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         kDefG,
         kDefC},
        {"07 Skia DW A8 (sys DW g/c)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         true,
         kDefG,
         kDefC},
        {"08 Skia DW A8 (g=1.8 c=0.5)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.8f,
         0.5f},
        {"09 Skia DW A8 (g=1.8 c=0.25)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.8f,
         0.25f},
        {"10 Skia DW A8 (g=1.8 c=0.0)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.8f,
         0.0f},
        {"11 Skia DW A8 (g=1.8 c=1.0)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.8f,
         1.0f},
        {"12 Skia DW A8 (g=1.6 c=0.5)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.6f,
         0.5f},
        {"13 Skia DW A8 (g=1.6 c=0.25)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.6f,
         0.25f},
        {"14 Skia DW A8 (g=1.4 c=0.5)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.4f,
         0.5f},
        {"15 Skia DW A8 (g=1.4 c=0.25)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.4f,
         0.25f},
        {"16 Skia DW A8 (g=1.4 c=0.0 Android)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.4f,
         0.0f},
        {"17 Skia DW A8 (g=1.0 c=0.0 raw)",
         RowKind::kSkia,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.0f,
         0.0f},
        {"18 Fallback en-US (sRGB c=0.5)",
         RowKind::kSkiaFallbackEnUs,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         kDefG,
         kDefC},
        {"19 Fallback en-US (g=1.4 c=0.5)",
         RowKind::kSkiaFallbackEnUs,
         false,
         kA8,
         SkFontHinting::kNormal,
         true,
         false,
         1.4f,
         0.5f},
};

// Reproduces MaterialApp's default Segoe UI + Locale('en', 'US') per-codepoint fallback chain
void draw_skia_fallback_line(SkCanvas* rc,
                             const char* utf8,
                             SkScalar baselineY,
                             const sk_sp<SkFontMgr>& mgr,
                             const sk_sp<SkTypeface>& primary,
                             const sk_sp<SkTypeface>& fallback,
                             const char* bcp47,
                             SkFont::Edging edging,
                             SkFontHinting hinting,
                             bool subpixel,
                             const SkPaint& paint) {
    const char* ptr = utf8;
    const char* end = utf8 + std::strlen(utf8);
    SkScalar x = 0;
    for (size_t guard = 0; ptr < end && guard < 1024; ++guard) {
        const char* prev = ptr;
        SkUnichar cp = SkUTF::NextUTF8(&ptr, end);
        if (cp < 0 || ptr <= prev) {
            break;
        }
        sk_sp<SkTypeface> tf = primary;
        if (!tf || tf->unicharToGlyph(cp) == 0) {
            if (mgr) {
                tf = mgr->matchFamilyStyleCharacter("Segoe UI", SkFontStyle(), &bcp47, 1, cp);
            }
            if (!tf) {
                tf = fallback;
            }
        }
        if (!tf) {
            continue;
        }
        SkFont font(tf, 14);
        font.setEdging(edging);
        font.setHinting(hinting);
        font.setSubpixel(subpixel);
        SkGlyphID glyph = font.unicharToGlyph(cp);
        SkPoint pos = {x, baselineY};
        rc->drawGlyphs({&glyph, 1}, {&pos, 1}, {0, 0}, font, paint);
        x += font.getWidth(glyph);
    }
}

#if defined(SK_BUILD_FOR_WIN)

std::vector<WCHAR> utf8_to_wide(const char* utf8) {
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (n <= 0) {
        return {0};
    }
    std::vector<WCHAR> out(static_cast<size_t>(n));
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, out.data(), n);
    return out;
}

struct TopDownDib {
    HDC fHdc = nullptr;
    HBITMAP fBitmap = nullptr;
    HGDIOBJ fOrigBitmap = nullptr;
    uint32_t* fPixels = nullptr;

    // GDI 32-bit BI_RGB DIB pixels are 0x00RRGGBB in memory, whereas Win32 COLORREF is 0x00BBGGRR
    bool init(int w, int h, uint32_t bgDibPixel) {
        fHdc = CreateCompatibleDC(nullptr);
        if (!fHdc) {
            return false;
        }
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        fBitmap = CreateDIBSection(fHdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!fBitmap || !bits) {
            DeleteDC(fHdc);
            fHdc = nullptr;
            return false;
        }
        fPixels = static_cast<uint32_t*>(bits);
        std::fill_n(fPixels, static_cast<size_t>(w) * static_cast<size_t>(h), bgDibPixel);
        fOrigBitmap = SelectObject(fHdc, fBitmap);
        return true;
    }

    ~TopDownDib() {
        if (fHdc) {
            if (fOrigBitmap) {
                SelectObject(fHdc, fOrigBitmap);
            }
            if (fBitmap) {
                DeleteObject(fBitmap);
            }
            DeleteDC(fHdc);
        }
    }
};

// GDI 32-bit DIBs store 0x00RRGGBB with an undefined zero alpha byte, so force opaque RGBA
void draw_dib_to_canvas(
        const uint32_t* dibPixels, int w, int h, SkCanvas* c, SkScalar x, SkScalar y) {
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    for (int row = 0; row < h; ++row) {
        uint8_t* dst = static_cast<uint8_t*>(bmp.getAddr(0, row));
        const uint32_t* src = dibPixels + static_cast<size_t>(row) * static_cast<size_t>(w);
        for (int col = 0; col < w; ++col) {
            uint32_t px = src[col];
            dst[col * 4 + 0] = static_cast<uint8_t>((px >> 16) & 0xFFu);
            dst[col * 4 + 1] = static_cast<uint8_t>((px >> 8) & 0xFFu);
            dst[col * 4 + 2] = static_cast<uint8_t>(px & 0xFFu);
            dst[col * 4 + 3] = 0xFFu;
        }
    }
    bmp.setImmutable();
    c->drawImage(bmp.asImage(), x, y);
}

void draw_native_gdi_cell(SkCanvas* c,
                          SkScalar left,
                          SkScalar top,
                          const char* family,
                          BYTE quality,
                          const ThemeCol& theme) {
    TopDownDib dib;
    if (!dib.init(kColWidth, kRowHeight, theme.bgDibPixel())) {
        return;
    }
    std::vector<WCHAR> wFamily = utf8_to_wide(family);
    std::vector<WCHAR> wLine1 = utf8_to_wide(kLine1);
    std::vector<WCHAR> wLine2 = utf8_to_wide(kLine2);

    LOGFONTW lf = {};
    lf.lfHeight = -14;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = quality;
    std::wcsncpy(lf.lfFaceName, wFamily.data(), LF_FACESIZE - 1);

    HFONT hfont = CreateFontIndirectW(&lf);
    if (!hfont) {
        return;
    }
    SetGraphicsMode(dib.fHdc, GM_ADVANCED);
    SetBkMode(dib.fHdc, TRANSPARENT);
    SetTextAlign(dib.fHdc, TA_LEFT | TA_BASELINE);
    SetTextColor(dib.fHdc, theme.fgColorRef());
    HGDIOBJ origFont = SelectObject(dib.fHdc, hfont);

    ExtTextOutW(dib.fHdc,
                0,
                18,
                0,
                nullptr,
                wLine1.data(),
                static_cast<UINT>(std::wcslen(wLine1.data())),
                nullptr);
    ExtTextOutW(dib.fHdc,
                0,
                38,
                0,
                nullptr,
                wLine2.data(),
                static_cast<UINT>(std::wcslen(wLine2.data())),
                nullptr);
    GdiFlush();

    SelectObject(dib.fHdc, origFont);
    DeleteObject(hfont);
    draw_dib_to_canvas(dib.fPixels, kColWidth, kRowHeight, c, left, top);
}

#if defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)

struct SystemDwParams {
    float fGamma = 1.8f;
    float fContrast = 0.5f;
    float fGrayContrast = 1.0f;
    float fClearTypeLevel = 1.0f;
    int fPixelGeometry = 0;
    int fRenderingMode = 0;
};

SystemDwParams query_system_dw_params(const sk_sp<SkTypeface>& dwFace) {
    SystemDwParams out;
    if (!dwFace) {
        return out;
    }
    auto* dwTf = static_cast<DWriteFontTypeface*>(dwFace.get());
    IDWriteFactory* factory = dwTf->fFactory.get();
    if (!factory) {
        return out;
    }
    SkTScopedComPtr<IDWriteRenderingParams> defParams;
    if (FAILED(factory->CreateRenderingParams(&defParams)) || !defParams) {
        return out;
    }
    out.fGamma = defParams->GetGamma();
    out.fContrast = defParams->GetEnhancedContrast();
    out.fGrayContrast = out.fContrast;
    out.fClearTypeLevel = defParams->GetClearTypeLevel();
    out.fPixelGeometry = static_cast<int>(defParams->GetPixelGeometry());
    out.fRenderingMode = static_cast<int>(defParams->GetRenderingMode());

    SkTScopedComPtr<IDWriteRenderingParams1> defParams1;
    if (SUCCEEDED(defParams->QueryInterface(&defParams1)) && defParams1) {
        out.fGrayContrast = defParams1->GetGrayscaleEnhancedContrast();
    }
    return out;
}

void draw_dw_line(IDWriteBitmapRenderTarget* rt,
                  IDWriteFontFace* fontFace,
                  IDWriteRenderingParams* params,
                  const SkFont& measureFont,
                  DWRITE_MEASURING_MODE measuringMode,
                  const char* utf8,
                  float baselineY,
                  COLORREF fgColorRef) {
    size_t byteLen = std::strlen(utf8);
    int count = measureFont.countText(utf8, byteLen, SkTextEncoding::kUTF8);
    if (count <= 0) {
        return;
    }
    std::vector<SkGlyphID> glyphs(static_cast<size_t>(count));
    measureFont.textToGlyphs(utf8, byteLen, SkTextEncoding::kUTF8, glyphs);
    std::vector<SkScalar> advances(static_cast<size_t>(count));
    measureFont.getWidths(glyphs, advances);
    std::vector<DWRITE_GLYPH_OFFSET> offsets(static_cast<size_t>(count), {0.0f, 0.0f});

    DWRITE_GLYPH_RUN run = {};
    run.fontFace = fontFace;
    run.fontEmSize = 14.0f;
    run.glyphCount = static_cast<UINT32>(count);
    run.glyphIndices = glyphs.data();
    run.glyphAdvances = advances.data();
    run.glyphOffsets = offsets.data();
    run.isSideways = FALSE;
    run.bidiLevel = 0;

    RECT dirty = {};
    rt->DrawGlyphRun(0.0f, baselineY, measuringMode, &run, params, fgColorRef, &dirty);
}

void draw_native_dw_cell(SkCanvas* c,
                         SkScalar left,
                         SkScalar top,
                         const sk_sp<SkTypeface>& dwFace,
                         const SystemDwParams& sys,
                         DWRITE_RENDERING_MODE renderingMode,
                         DWRITE_MEASURING_MODE measuringMode,
                         DWRITE_TEXT_ANTIALIAS_MODE aaMode,
                         SkFontHinting hinting,
                         bool subpixel,
                         const ThemeCol& theme) {
    if (!dwFace) {
        return;
    }
    auto* dwTf = static_cast<DWriteFontTypeface*>(dwFace.get());
    IDWriteFactory* factory = dwTf->fFactory.get();
    IDWriteFontFace* fontFace = dwTf->fDWriteFontFace.get();
    if (!factory || !fontFace) {
        return;
    }

    SkTScopedComPtr<IDWriteGdiInterop> interop;
    if (FAILED(factory->GetGdiInterop(&interop)) || !interop) {
        return;
    }
    SkTScopedComPtr<IDWriteBitmapRenderTarget> rt;
    if (FAILED(interop->CreateBitmapRenderTarget(nullptr, kColWidth, kRowHeight, &rt)) || !rt) {
        return;
    }
    rt->SetPixelsPerDip(1.0f);
    SkTScopedComPtr<IDWriteBitmapRenderTarget1> rt1;
    if (SUCCEEDED(rt->QueryInterface(&rt1)) && rt1) {
        rt1->SetTextAntialiasMode(aaMode);
    }

    HDC rtDc = rt->GetMemoryDC();
    HBRUSH br = CreateSolidBrush(theme.bgColorRef());
    RECT fillRc = {0, 0, kColWidth, kRowHeight};
    FillRect(rtDc, &fillRc, br);
    DeleteObject(br);

    const bool isClearType = (aaMode == DWRITE_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    const FLOAT ctLevel = isClearType ? 1.0f : 0.0f;
    const DWRITE_PIXEL_GEOMETRY geom =
            isClearType ? DWRITE_PIXEL_GEOMETRY_RGB : DWRITE_PIXEL_GEOMETRY_FLAT;

    SkTScopedComPtr<IDWriteRenderingParams> params;
    SkTScopedComPtr<IDWriteFactory1> factory1;
    if (SUCCEEDED(factory->QueryInterface(&factory1)) && factory1) {
        SkTScopedComPtr<IDWriteRenderingParams1> params1;
        if (SUCCEEDED(factory1->CreateCustomRenderingParams(sys.fGamma,
                                                            sys.fContrast,
                                                            sys.fGrayContrast,
                                                            ctLevel,
                                                            geom,
                                                            renderingMode,
                                                            &params1))) {
            params.reset(params1.release());
        }
    }
    if (!params) {
        factory->CreateCustomRenderingParams(
                sys.fGamma, sys.fContrast, ctLevel, geom, renderingMode, &params);
    }
    if (!params) {
        return;
    }

    SkFont measureFont(dwFace, 14);
    measureFont.setEdging(isClearType ? kLCD : kA8);
    measureFont.setHinting(hinting);
    measureFont.setSubpixel(subpixel);

    draw_dw_line(rt.get(),
                 fontFace,
                 params.get(),
                 measureFont,
                 measuringMode,
                 kLine1,
                 18.0f,
                 theme.fgColorRef());
    draw_dw_line(rt.get(),
                 fontFace,
                 params.get(),
                 measureFont,
                 measuringMode,
                 kLine2,
                 38.0f,
                 theme.fgColorRef());
    GdiFlush();

    // Copy through a known top-down DIB so orientation does not depend on GetMemoryDC internals
    TopDownDib dib;
    if (!dib.init(kColWidth, kRowHeight, theme.bgDibPixel())) {
        return;
    }
    BitBlt(dib.fHdc, 0, 0, kColWidth, kRowHeight, rtDc, 0, 0, SRCCOPY);
    GdiFlush();
    draw_dib_to_canvas(dib.fPixels, kColWidth, kRowHeight, c, left, top);
}

#endif  // SK_FONTMGR_DIRECTWRITE_AVAILABLE
#endif  // SK_BUILD_FOR_WIN

// Gold and the GCS image bucket need extra access, so emit the PNG once without thread interleaving
void dump_png_to_log_once(const SkPixmap& pixmap) {
    static std::atomic<bool> gDumped{false};
    if (gDumped.exchange(true)) {
        return;
    }
    sk_sp<SkData> png = SkPngEncoder::Encode(pixmap, {});
    if (!png) {
        SkDebugf("WIN_CJK_PNG_FAILED\n");
        return;
    }
    std::vector<char> b64(SkBase64::EncodedSize(png->size()));
    size_t len = SkBase64::Encode(png->data(), png->size(), b64.data());
    constexpr size_t kChunk = 512;
    SkDebugf("WIN_CJK_PNG_BEGIN %zu\n", len);
    for (size_t i = 0; i < len; i += kChunk) {
        size_t n = std::min(kChunk, len - i);
        SkDebugf("WIN_CJK_PNG %.*s\n", static_cast<int>(n), b64.data() + i);
    }
    SkDebugf("WIN_CJK_PNG_END\n");
}

}  // namespace

DEF_SIMPLE_GM_BG(win_cjk_crispness, canvas, kWidth, kHeight, SK_ColorWHITE) {
    sk_sp<SkFontMgr> dwMgr;
    sk_sp<SkFontMgr> gdiMgr;
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
    dwMgr = SkFontMgr_New_DirectWrite();
#endif
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
    gdiMgr = SkFontMgr_New_GDI();
#endif
    SkString dwFamily("none");
    SkString gdiFamily("none");
    sk_sp<SkTypeface> dwFace = match_cjk(dwMgr, &dwFamily);
    sk_sp<SkTypeface> gdiFace = match_cjk(gdiMgr, &gdiFamily);
    sk_sp<SkTypeface> segoeFace =
            dwMgr ? dwMgr->matchFamilyStyle("Segoe UI", SkFontStyle()) : nullptr;

    SkString enUsNi = fallback_family_name(dwMgr, "Segoe UI", "en-US", 1, 0x4F60);
    SkString enUsShu = fallback_family_name(dwMgr, "Segoe UI", "en-US", 1, 0x6811);
    SkString zhBareNi = fallback_family_name(dwMgr, "Segoe UI", "zh", 1, 0x4F60);
    SkString zhCnNi = fallback_family_name(dwMgr, "Segoe UI", "zh-CN", 1, 0x4F60);
    SkString zhHansNi = fallback_family_name(dwMgr, "Segoe UI", "zh-Hans", 1, 0x4F60);
    SkString sysLocNi = fallback_family_name(dwMgr, "Segoe UI", nullptr, 0, 0x4F60);

    SkScalar sysGamma = 1.8f;
    SkScalar sysContrast = 0.5f;
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
    SystemDwParams sys = query_system_dw_params(dwFace);
    // Row 07 mirrors SK_FONT_HOST_USE_SYSTEM_SETTINGS (GetGamma + GetEnhancedContrast)
    sysGamma = std::clamp(sys.fGamma, 0.0f, 3.9f);
    sysContrast = std::clamp(sys.fContrast, 0.0f, 1.0f);
    static std::atomic<bool> gLoggedParams{false};
    if (!gLoggedParams.exchange(true)) {
        SkDebugf(
                "WIN_CJK_DW_PARAMS gamma=%.3f contrast=%.3f grayContrast=%.3f ctLevel=%.3f "
                "geom=%d mode=%d dwFamily=%s gdiFamily=%s\n",
                sys.fGamma,
                sys.fContrast,
                sys.fGrayContrast,
                sys.fClearTypeLevel,
                sys.fPixelGeometry,
                sys.fRenderingMode,
                dwFamily.c_str(),
                gdiFamily.c_str());
        SkDebugf(
                "WIN_CJK_FALLBACK en_US_ni=%s en_US_shu=%s zh_bare=%s zh_CN=%s zh_Hans=%s "
                "sys_loc=%s\n",
                enUsNi.c_str(),
                enUsShu.c_str(),
                zhBareNi.c_str(),
                zhCnNi.c_str(),
                zhHansNi.c_str(),
                sysLocNi.c_str());
    }
#endif

    // LCD text needs a surface with a known pixel geometry; the GM canvas has none
    SkSurfaceProps baseProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface =
            SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kWidth, kHeight), &baseProps);
    SkCanvas* c = surface->getCanvas();
    c->clear(SK_ColorWHITE);

    SkPaint labelPaint;
    labelPaint.setColor(SK_ColorBLACK);
    SkFont labelFont = ToolUtils::DefaultPortableFont();
    labelFont.setSize(10.5f);

    for (int col = 0; col < kCols; ++col) {
        const SkScalar colLeft = SkIntToScalar(kLabelWidth + col * kColWidth);
        c->drawString(kThemes[col].fTitle, colLeft + 4, 16, labelFont, labelPaint);
    }

    for (int i = 0; i < kRows; ++i) {
        const Variant& v = kVariants[i];
        const SkScalar top = SkIntToScalar(kHeaderHeight + i * kRowHeight);
        const sk_sp<SkTypeface>& face = v.fGdi ? gdiFace : dwFace;

        SkString subLabel;
        if (v.fUseSysDwGamma) {
            subLabel.printf("[sys g=%.2f c=%.2f, %s]", sysGamma, sysContrast, dwFamily.c_str());
        } else if (v.fKind == RowKind::kSkiaFallbackEnUs) {
            subLabel.printf("[Segoe UI -> %s / %s]", enUsNi.c_str(), enUsShu.c_str());
        } else {
            subLabel.printf("[%s]", v.fGdi ? gdiFamily.c_str() : dwFamily.c_str());
        }
        c->save();
        c->clipRect(SkRect::MakeXYWH(
                0, top, SkIntToScalar(kLabelWidth - 4), SkIntToScalar(kRowHeight)));
        c->drawString(v.fName, 4, top + 17, labelFont, labelPaint);
        c->drawString(subLabel.c_str(), 4, top + 34, labelFont, labelPaint);
        c->restore();

        for (int col = 0; col < kCols; ++col) {
            const ThemeCol& theme = kThemes[col];
            const SkScalar left = SkIntToScalar(kLabelWidth + col * kColWidth);

            if (v.fKind == RowKind::kSkia || v.fKind == RowKind::kSkiaFallbackEnUs) {
                if (!face && v.fKind == RowKind::kSkia) {
                    continue;
                }
                SkScalar rowGamma = v.fUseSysDwGamma ? sysGamma : v.fGamma;
                SkScalar rowContrast = v.fUseSysDwGamma ? sysContrast : v.fContrast;
                SkSurfaceProps rowProps(0, kRGB_H_SkPixelGeometry, rowContrast, rowGamma);
                sk_sp<SkSurface> cellSurf = SkSurfaces::Raster(
                        SkImageInfo::MakeN32Premul(kColWidth, kRowHeight), &rowProps);
                SkCanvas* rc = cellSurf->getCanvas();
                rc->clear(theme.bgSkColor());

                SkPaint fgPaint;
                fgPaint.setColor(theme.fgSkColor());
                if (v.fKind == RowKind::kSkiaFallbackEnUs) {
                    draw_skia_fallback_line(rc,
                                            kLine1,
                                            18,
                                            dwMgr,
                                            segoeFace,
                                            dwFace,
                                            "en-US",
                                            v.fEdging,
                                            v.fHinting,
                                            v.fSubpixel,
                                            fgPaint);
                    draw_skia_fallback_line(rc,
                                            kLine2,
                                            38,
                                            dwMgr,
                                            segoeFace,
                                            dwFace,
                                            "en-US",
                                            v.fEdging,
                                            v.fHinting,
                                            v.fSubpixel,
                                            fgPaint);
                } else {
                    SkFont font(face, 14);
                    font.setEdging(v.fEdging);
                    font.setHinting(v.fHinting);
                    font.setSubpixel(v.fSubpixel);
                    rc->drawString(kLine1, 0, 18, font, fgPaint);
                    rc->drawString(kLine2, 0, 38, font, fgPaint);
                }
                c->drawImage(cellSurf->makeImageSnapshot(), left, top);
                continue;
            }

#if defined(SK_BUILD_FOR_WIN)
            if (v.fKind == RowKind::kNativeGdiClearType && gdiFace) {
                draw_native_gdi_cell(c, left, top, gdiFamily.c_str(), CLEARTYPE_QUALITY, theme);
            } else if (v.fKind == RowKind::kNativeGdiAntialiased && gdiFace) {
                draw_native_gdi_cell(c, left, top, gdiFamily.c_str(), ANTIALIASED_QUALITY, theme);
            }
#if defined(SK_FONTMGR_DIRECTWRITE_AVAILABLE)
            else if (v.fKind == RowKind::kNativeDwClearTypeDefault) {
                draw_native_dw_cell(c,
                                    left,
                                    top,
                                    dwFace,
                                    sys,
                                    DWRITE_RENDERING_MODE_DEFAULT,
                                    DWRITE_MEASURING_MODE_NATURAL,
                                    DWRITE_TEXT_ANTIALIAS_MODE_CLEARTYPE,
                                    v.fHinting,
                                    v.fSubpixel,
                                    theme);
            } else if (v.fKind == RowKind::kNativeDwClearTypeGdiClassic) {
                draw_native_dw_cell(c,
                                    left,
                                    top,
                                    dwFace,
                                    sys,
                                    DWRITE_RENDERING_MODE_GDI_CLASSIC,
                                    DWRITE_MEASURING_MODE_GDI_CLASSIC,
                                    DWRITE_TEXT_ANTIALIAS_MODE_CLEARTYPE,
                                    v.fHinting,
                                    v.fSubpixel,
                                    theme);
            } else if (v.fKind == RowKind::kNativeDwGrayscaleDefault) {
                draw_native_dw_cell(c,
                                    left,
                                    top,
                                    dwFace,
                                    sys,
                                    DWRITE_RENDERING_MODE_DEFAULT,
                                    DWRITE_MEASURING_MODE_NATURAL,
                                    DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE,
                                    v.fHinting,
                                    v.fSubpixel,
                                    theme);
            } else if (v.fKind == RowKind::kNativeDwGrayscaleNatSym) {
                draw_native_dw_cell(c,
                                    left,
                                    top,
                                    dwFace,
                                    sys,
                                    DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
                                    DWRITE_MEASURING_MODE_NATURAL,
                                    DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE,
                                    v.fHinting,
                                    v.fSubpixel,
                                    theme);
            }
#endif  // SK_FONTMGR_DIRECTWRITE_AVAILABLE
#endif  // SK_BUILD_FOR_WIN
        }
    }

    SkPixmap pixmap;
    if (surface->peekPixels(&pixmap)) {
        dump_png_to_log_once(pixmap);
    }
    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
}
