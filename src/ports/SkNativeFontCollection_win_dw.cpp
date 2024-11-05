/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkTypeface_win.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkEndian.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteFontFileStream.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkObjBase.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

using namespace skia_private;

namespace {

// Korean fonts Gulim, Dotum, Batang, Gungsuh have bitmap strikes that get
// artifically emboldened by Windows without antialiasing. Korean users prefer
// these over the synthetic boldening performed by Skia. So let's make an
// exception for fonts with bitmap strikes and allow passing through Windows
// simulations for those, until Skia provides more control over simulations in
// font matching, see https://crbug.com/1258378
bool HasBitmapStrikes(const SkTScopedComPtr<IDWriteFont>& font) {
  SkTScopedComPtr<IDWriteFontFace> fontFace;
  HRB(font->CreateFontFace(&fontFace));

  AutoDWriteTable ebdtTable(fontFace.get(),
                            SkEndian_SwapBE32(SkSetFourByteTag('E', 'B', 'D', 'T')));
  return ebdtTable.fExists;
}

// Iterate calls to GetFirstMatchingFont incrementally removing bold or italic
// styling that can trigger the simulations. Implementing it this way gets us a
// IDWriteFont that can be used as before and has the correct information on its
// own style. Stripping simulations from IDWriteFontFace is possible via
// IDWriteFontList1, IDWriteFontFaceReference and CreateFontFace, but this way
// we won't have a matching IDWriteFont which is still used in get_style().
HRESULT FirstMatchingFontWithoutSimulations(IDWriteFontFamily* family,
                                            DWriteStyle dwStyle,
                                            SkTScopedComPtr<IDWriteFont>& font) {
    bool noSimulations = false;
    while (!noSimulations) {
        SkTScopedComPtr<IDWriteFont> searchFont;
        HR(family->GetFirstMatchingFont(
                dwStyle.fWeight, dwStyle.fWidth, dwStyle.fSlant, &searchFont));
        DWRITE_FONT_SIMULATIONS simulations = searchFont->GetSimulations();
        // If we still get simulations even though we're not asking for bold or
        // italic, we can't help it and exit the loop.

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = simulations == DWRITE_FONT_SIMULATIONS_NONE ||
                        (dwStyle.fWeight == DWRITE_FONT_WEIGHT_REGULAR &&
                         dwStyle.fSlant == DWRITE_FONT_STYLE_NORMAL) ||
                        HasBitmapStrikes(searchFont);
#else
        noSimulations = true;
#endif
        if (noSimulations) {
            font = std::move(searchFont);
            break;
        }
        if (simulations & DWRITE_FONT_SIMULATIONS_BOLD) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }
        if (simulations & DWRITE_FONT_SIMULATIONS_OBLIQUE) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }
    return S_OK;
}
}

////////////////////////////////////////////////////////////////////////////////

class SkNativeFontCollection_DirectWrite_impl: public SkNativeFontCollection_DirectWrite {
public:
    int countFamilies() const override;
    void getFamilyName(int index, SkString* familyName) const override;
    IDWriteFontFamily* createStyleSet(int index) const override;
    IDWriteFontFamily* matchFamily(const char familyName[]) const override;
    Match matchFamilyStyle(IDWriteFontFamily*,
                           const SkFontStyle& fontstyle) const override;
    Match matchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                    const char* bcp47[], int bcp47Count,
                                    const WCHAR* str, UINT32 strLen) const override;
    Match legacyMakeTypeface(const char familyName[], SkFontStyle) const override;

    IDWriteFactory* factory() const override { return fFactory.get(); }
    WCHAR* locale() const override { return fLocaleName.get(); }

    /** localeNameLength and defaultFamilyNameLength must include the null terminator. */
    SkNativeFontCollection_DirectWrite_impl(IDWriteFactory* factory,
                                            IDWriteFontCollection* fontCollection,
                                            IDWriteFontFallback* fallback,
                                            const WCHAR* localeName, int localeNameLength,
                                            const WCHAR* defaultFamilyName, int defaultFamilyNameLength);

private:
    HRESULT getByFamilyName(const WCHAR familyName[], IDWriteFontFamily** fontFamily) const;
    Match fallback(const WCHAR* dwFamilyName, DWriteStyle,
                   const WCHAR* dwBcp47, const WCHAR* str, UINT32 strLen) const;
    Match layoutFallback(const WCHAR* dwFamilyName, DWriteStyle,
                         const WCHAR* dwBcp47, const WCHAR* str, UINT32 strLen) const;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontFallback> fFontFallback;
    SkTScopedComPtr<IDWriteFontCollection> fFontCollection;
    SkSMallocWCHAR fLocaleName;
    SkSMallocWCHAR fDefaultFamilyName;

    friend class SkFontMgr_DirectWrite;
    friend class SkFontStyleSet_DirectWrite;
    friend class FontFallbackRenderer;
};


/** localeNameLength and defaultFamilyNameLength must include the null terminator. */
SkNativeFontCollection_DirectWrite_impl::SkNativeFontCollection_DirectWrite_impl(
        IDWriteFactory* factory, IDWriteFontCollection* fontCollection,
        IDWriteFontFallback* fallback,
        const WCHAR* localeName, int localeNameLength,
        const WCHAR* defaultFamilyName, int defaultFamilyNameLength)
    : fFactory(SkRefComPtr(factory))
    , fFontFallback(SkSafeRefComPtr(fallback))
    , fFontCollection(SkRefComPtr(fontCollection))
    , fLocaleName(localeNameLength)
    , fDefaultFamilyName(defaultFamilyNameLength)
{
    memcpy(fLocaleName.get(), localeName, localeNameLength * sizeof(WCHAR));
    memcpy(fDefaultFamilyName.get(), defaultFamilyName, defaultFamilyNameLength*sizeof(WCHAR));
}

int SkNativeFontCollection_DirectWrite_impl::countFamilies() const {
    return fFontCollection->GetFontFamilyCount();
}

void SkNativeFontCollection_DirectWrite_impl::getFamilyName(int index, SkString* familyName) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRVM(fFontCollection->GetFontFamily(index, &fontFamily), "Could not get requested family.");

    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRVM(fontFamily->GetFamilyNames(&familyNames), "Could not get family names.");

    sk_get_locale_string(familyNames.get(), fLocaleName.get(), familyName);
}

IDWriteFontFamily* SkNativeFontCollection_DirectWrite_impl::createStyleSet(int index) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRNM(fFontCollection->GetFontFamily(index, &fontFamily), "Could not get requested family.");
    return fontFamily.release();
}

IDWriteFontFamily* SkNativeFontCollection_DirectWrite_impl::matchFamily(const char familyName[]) const {
    if (!familyName) {
        return nullptr;
    }

    SkSMallocWCHAR dwFamilyName;
    HRN(sk_cstring_to_wchar(familyName, &dwFamilyName));

    UINT32 index;
    BOOL exists;
    HRNM(fFontCollection->FindFamilyName(dwFamilyName.get(), &index, &exists),
            "Failed while finding family by name.");
    if (!exists) {
        return nullptr;
    }

    return this->createStyleSet(index);
}

auto SkNativeFontCollection_DirectWrite_impl::matchFamilyStyle(IDWriteFontFamily* sset, const SkFontStyle& pattern) const -> Match {
    if (!sset) {
        return nullptr;
    }
    SkTScopedComPtr<IDWriteFont> font;
    DWriteStyle dwStyle(pattern);

    HRNM(FirstMatchingFontWithoutSimulations(sset, dwStyle, font),
         "No font found from family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return Match(SkRefComPtr(sset), font.release(), fontFace.release());
}

class FontFallbackRenderer : public IDWriteTextRenderer {
public:
    FontFallbackRenderer(const SkNativeFontCollection_DirectWrite_impl* outer, UINT32 character)
        : fRefCount(1), fOuter(SkSafeRef(outer)), fCharacter(character), fResolvedMatch(nullptr) {
    }

    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(IID const& riid, void** ppvObject) override {
        if (__uuidof(IUnknown) == riid ||
            __uuidof(IDWritePixelSnapping) == riid ||
            __uuidof(IDWriteTextRenderer) == riid)
        {
            *ppvObject = this;
            this->AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_FAIL;
    }

    SK_STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&fRefCount);
    }

    SK_STDMETHODIMP_(ULONG) Release() override {
        ULONG newCount = InterlockedDecrement(&fRefCount);
        if (0 == newCount) {
            delete this;
        }
        return newCount;
    }

    // IDWriteTextRenderer methods
    SK_STDMETHODIMP DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect) override
    {
        if (!glyphRun->fontFace) {
            HRM(E_INVALIDARG, "Glyph run without font face.");
        }

        SkTScopedComPtr<IDWriteFont> font;
        HRM(fOuter->fFontCollection->GetFontFromFontFace(glyphRun->fontFace, &font),
            "Could not get font from font face.");

        // It is possible that the font passed does not actually have the requested character,
        // due to no font being found and getting the fallback font.
        // Check that the font actually contains the requested character.
        BOOL exists;
        HRM(font->HasCharacter(fCharacter, &exists), "Could not find character.");

        if (exists) {
            SkTScopedComPtr<IDWriteFontFamily> fontFamily;
            HRM(font->GetFontFamily(&fontFamily), "Could not get family.");
            fResolvedMatch = SkNativeFontCollection_DirectWrite::Match(fontFamily.release(), font.release(), SkRefComPtr(glyphRun->fontFace));
            fHasSimulations = (font->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE) &&
                              !HasBitmapStrikes(font);
        }

        return S_OK;
    }

    SK_STDMETHODIMP DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    SK_STDMETHODIMP DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    SK_STDMETHODIMP DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    // IDWritePixelSnapping methods
    SK_STDMETHODIMP IsPixelSnappingDisabled(
        void* clientDrawingContext,
        BOOL* isDisabled) override
    {
        *isDisabled = FALSE;
        return S_OK;
    }

    SK_STDMETHODIMP GetCurrentTransform(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform) override
    {
        const DWRITE_MATRIX ident = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
        *transform = ident;
        return S_OK;
    }

    SK_STDMETHODIMP GetPixelsPerDip(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip) override
    {
        *pixelsPerDip = 1.0f;
        return S_OK;
    }

    SkNativeFontCollection_DirectWrite::Match ConsumeFallbackTypeface() { return std::move(fResolvedMatch); }

    bool FallbackTypefaceHasSimulations() const { return fHasSimulations; }

private:
    virtual ~FontFallbackRenderer() { }

    ULONG fRefCount;
    sk_sp<const SkNativeFontCollection_DirectWrite_impl> fOuter; // TODO: just IDWriteFontCollection now.
    UINT32 fCharacter;
    SkNativeFontCollection_DirectWrite::Match fResolvedMatch;
    bool fHasSimulations{false};
};

class FontFallbackSource : public IDWriteTextAnalysisSource {
public:
    FontFallbackSource(const WCHAR* string, UINT32 length, const WCHAR* locale,
                       IDWriteNumberSubstitution* numberSubstitution)
        : fRefCount(1)
        , fString(string)
        , fLength(length)
        , fLocale(locale)
        , fNumberSubstitution(numberSubstitution)
    { }

    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(IID const& riid, void** ppvObject) override {
        if (__uuidof(IUnknown) == riid ||
            __uuidof(IDWriteTextAnalysisSource) == riid)
        {
            *ppvObject = this;
            this->AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_FAIL;
    }

    SK_STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&fRefCount);
    }

    SK_STDMETHODIMP_(ULONG) Release() override {
        ULONG newCount = InterlockedDecrement(&fRefCount);
        if (0 == newCount) {
            delete this;
        }
        return newCount;
    }

    // IDWriteTextAnalysisSource methods
    SK_STDMETHODIMP GetTextAtPosition(
        UINT32 textPosition,
        WCHAR const** textString,
        UINT32* textLength) override
    {
        if (fLength <= textPosition) {
            *textString = nullptr;
            *textLength = 0;
            return S_OK;
        }
        *textString = fString + textPosition;
        *textLength = fLength - textPosition;
        return S_OK;
    }

    SK_STDMETHODIMP GetTextBeforePosition(
        UINT32 textPosition,
        WCHAR const** textString,
        UINT32* textLength) override
    {
        if (textPosition < 1 || fLength <= textPosition) {
            *textString = nullptr;
            *textLength = 0;
            return S_OK;
        }
        *textString = fString;
        *textLength = textPosition;
        return S_OK;
    }

    SK_STDMETHODIMP_(DWRITE_READING_DIRECTION) GetParagraphReadingDirection() override {
        // TODO: this is also interesting.
        return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
    }

    SK_STDMETHODIMP GetLocaleName(
        UINT32 textPosition,
        UINT32* textLength,
        WCHAR const** localeName) override
    {
        *localeName = fLocale;
        return S_OK;
    }

    SK_STDMETHODIMP GetNumberSubstitution(
        UINT32 textPosition,
        UINT32* textLength,
        IDWriteNumberSubstitution** numberSubstitution) override
    {
        *numberSubstitution = fNumberSubstitution;
        return S_OK;
    }

private:
    virtual ~FontFallbackSource() { }

    ULONG fRefCount;
    const WCHAR* fString;
    UINT32 fLength;
    const WCHAR* fLocale;
    IDWriteNumberSubstitution* fNumberSubstitution;
};

auto SkNativeFontCollection_DirectWrite_impl::matchFamilyStyleCharacter(
    const char familyName[], const SkFontStyle& style,
    const char* bcp47[], int bcp47Count,
    const WCHAR* str, UINT32 strLen) const -> Match
{
    DWriteStyle dwStyle(style);

    const WCHAR* dwFamilyName = nullptr;
    SkSMallocWCHAR dwFamilyNameLocal;
    if (familyName) {
        HRN(sk_cstring_to_wchar(familyName, &dwFamilyNameLocal));
        dwFamilyName = dwFamilyNameLocal;
    }

    const SkSMallocWCHAR* dwBcp47;
    SkSMallocWCHAR dwBcp47Local;
    if (bcp47Count < 1) {
        dwBcp47 = &fLocaleName;
    } else {
        // TODO: support fallback stack.
        // TODO: DirectWrite supports 'zh-CN' or 'zh-Hans', but 'zh' misses completely
        // and may produce a Japanese font.
        HRN(sk_cstring_to_wchar(bcp47[bcp47Count - 1], &dwBcp47Local));
        dwBcp47 = &dwBcp47Local;
    }

    if (fFontFallback) {
        return this->fallback(dwFamilyName, dwStyle, dwBcp47->get(), str, strLen);
    }

    // LayoutFallback may use the system font collection for fallback.
    return this->layoutFallback(dwFamilyName, dwStyle, dwBcp47->get(), str, strLen);
}

auto SkNativeFontCollection_DirectWrite_impl::fallback(const WCHAR* dwFamilyName,
                                                  DWriteStyle dwStyle,
                                                  const WCHAR* dwBcp47,
                                                  const WCHAR* str, UINT32 strLen) const -> Match {
    if (!fFontFallback) {
        return nullptr;
    }

    SkTScopedComPtr<IDWriteNumberSubstitution> numberSubstitution;
    HRNM(fFactory->CreateNumberSubstitution(DWRITE_NUMBER_SUBSTITUTION_METHOD_NONE, dwBcp47,
                                            TRUE, &numberSubstitution),
         "Could not create number substitution.");
    SkTScopedComPtr<FontFallbackSource> fontFallbackSource(
        new FontFallbackSource(str, strLen, dwBcp47, numberSubstitution.get()));

    UINT32 mappedLength;
    SkTScopedComPtr<IDWriteFont> font;
    FLOAT scale;

    bool noSimulations = false;
    while (!noSimulations) {
        font.reset();
        HRNM(fFontFallback->MapCharacters(fontFallbackSource.get(),
                                          0,  // textPosition,
                                          strLen,
                                          fFontCollection.get(),
                                          dwFamilyName,
                                          dwStyle.fWeight,
                                          dwStyle.fSlant,
                                          dwStyle.fWidth,
                                          &mappedLength,
                                          &font,
                                          &scale),
             "Could not map characters");
        if (!font.get()) {
            return nullptr;
        }

        DWRITE_FONT_SIMULATIONS simulations = font->GetSimulations();

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = simulations == DWRITE_FONT_SIMULATIONS_NONE || HasBitmapStrikes(font);
#else
        noSimulations = true;
#endif

        if (simulations & DWRITE_FONT_SIMULATIONS_BOLD) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }

        if (simulations & DWRITE_FONT_SIMULATIONS_OBLIQUE) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not get font face from font.");

    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRNM(font->GetFontFamily(&fontFamily), "Could not get family from font.");
    return Match(fontFamily.release(), font.release(), fontFace.release());
}

auto SkNativeFontCollection_DirectWrite_impl::layoutFallback(const WCHAR* dwFamilyName,
                                                        DWriteStyle dwStyle,
                                                        const WCHAR* dwBcp47,
                                                        const WCHAR* str, UINT32 strLen) const -> Match
{
    bool noSimulations = false;
    Match match(nullptr);
    while (!noSimulations) {
        SkTScopedComPtr<IDWriteTextFormat> fallbackFormat;
        HRNM(fFactory->CreateTextFormat(dwFamilyName ? dwFamilyName : L"",
                                        fFontCollection.get(),
                                        dwStyle.fWeight,
                                        dwStyle.fSlant,
                                        dwStyle.fWidth,
                                        72.0f,
                                        dwBcp47,
                                        &fallbackFormat),
             "Could not create text format.");

        // No matter how the font collection is set on this IDWriteTextLayout, it is not possible to
        // disable use of the system font collection in fallback.
        SkTScopedComPtr<IDWriteTextLayout> fallbackLayout;
        HRNM(fFactory->CreateTextLayout(
                     str, strLen, fallbackFormat.get(), 200.0f, 200.0f, &fallbackLayout),
             "Could not create text layout.");

        const uint16_t* begin = reinterpret_cast<const uint16_t*>(str);
        SkUnichar character = SkUTF::NextUTF16(&begin, begin + strLen);
        if (character < 0) {
            return nullptr;
        }
        SkTScopedComPtr<FontFallbackRenderer> fontFallbackRenderer(
                new FontFallbackRenderer(this, character));

        HRNM(fallbackLayout->SetFontCollection(fFontCollection.get(), {0, strLen}),
             "Could not set layout font collection.");
        HRNM(fallbackLayout->Draw(nullptr, fontFallbackRenderer.get(), 50.0f, 50.0f),
             "Could not draw layout with renderer.");

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = !fontFallbackRenderer->FallbackTypefaceHasSimulations();
#else
        noSimulations = true;
#endif

        if (noSimulations) {
            match = fontFallbackRenderer->ConsumeFallbackTypeface();
        }

        if (dwStyle.fWeight != DWRITE_FONT_WEIGHT_REGULAR) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }

        if (dwStyle.fSlant != DWRITE_FONT_STYLE_NORMAL) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }

    return match;
}

HRESULT SkNativeFontCollection_DirectWrite_impl::getByFamilyName(const WCHAR wideFamilyName[],
                                               IDWriteFontFamily** fontFamily) const {
    UINT32 index;
    BOOL exists;
    HR(fFontCollection->FindFamilyName(wideFamilyName, &index, &exists));

    if (exists) {
        HR(fFontCollection->GetFontFamily(index, fontFamily));
    }
    return S_OK;
}

auto SkNativeFontCollection_DirectWrite_impl::legacyMakeTypeface(const char familyName[],
                                                              SkFontStyle style) const -> Match {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    DWriteStyle dwStyle(style);
    if (familyName) {
        SkSMallocWCHAR dwFamilyName;
        if (SUCCEEDED(sk_cstring_to_wchar(familyName, &dwFamilyName))) {
            this->getByFamilyName(dwFamilyName, &fontFamily);
            if (!fontFamily && fFontFallback) {
                return this->fallback(
                        dwFamilyName, dwStyle, fLocaleName.get(), L" ", 1);
            }
        }
    }

    if (!fontFamily) {
        if (fFontFallback) {
            return this->fallback(nullptr, dwStyle, fLocaleName.get(), L" ", 1);
        }
        // SPI_GETNONCLIENTMETRICS lfMessageFont can fail in Win8. (DisallowWin32kSystemCalls)
        // layoutFallback causes DCHECK in Chromium. (Uses system font collection.)
        HRNM(this->getByFamilyName(fDefaultFamilyName, &fontFamily),
             "Could not create DWrite font family from LOGFONT.");
    }

    if (!fontFamily) {
        // Could not obtain the default font.
        HRNM(fFontCollection->GetFontFamily(0, &fontFamily),
             "Could not get default-default font family.");
    }

    SkTScopedComPtr<IDWriteFont> font;
    HRNM(FirstMatchingFontWithoutSimulations(fontFamily.get(), dwStyle, font),
         "No font found from family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return Match(fontFamily.release(), font.release(), fontFace.release());
}

///////////////////////////////////////////////////////////////////////////////

#include "include/ports/SkTypeface_win.h"

sk_sp<SkNativeFontCollection_DirectWrite> SkNativeFontCollection_DirectWrite::Make(IDWriteFactory* factory,
                                                    IDWriteFontCollection* collection,
                                                    IDWriteFontFallback* fallback) {
    if (nullptr == factory) {
        factory = sk_get_dwrite_factory();
        if (nullptr == factory) {
            return nullptr;
        }
    }

    SkTScopedComPtr<IDWriteFontCollection> systemFontCollection;
    if (nullptr == collection) {
        HRNM(factory->GetSystemFontCollection(&systemFontCollection, FALSE),
             "Could not get system font collection.");
        collection = systemFontCollection.get();
    }

    // It is possible to have been provided a font fallback when factory2 is not available.
    SkTScopedComPtr<IDWriteFontFallback> systemFontFallback;
    if (nullptr == fallback) {
        SkTScopedComPtr<IDWriteFactory2> factory2;
        if (!SUCCEEDED(factory->QueryInterface(&factory2))) {
            // IUnknown::QueryInterface states that if it fails, punk will be set to nullptr.
            // http://blogs.msdn.com/b/oldnewthing/archive/2004/03/26/96777.aspx
            SkASSERT_RELEASE(nullptr == factory2.get());
        } else {
            HRNM(factory2->GetSystemFontFallback(&systemFontFallback),
                 "Could not get system fallback.");
            fallback = systemFontFallback.get();
        }
    }

    const WCHAR* defaultFamilyName = L"";
    int defaultFamilyNameLen = 1;
    NONCLIENTMETRICSW metrics;
    metrics.cbSize = sizeof(metrics);

    #ifndef SK_WINUWP
    if (nullptr == fallback) {
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
            defaultFamilyName = metrics.lfMessageFont.lfFaceName;
            defaultFamilyNameLen = LF_FACESIZE;
        }
    }
    #endif //SK_WINUWP

    WCHAR localeNameStorage[LOCALE_NAME_MAX_LENGTH];
    const WCHAR* localeName = L"";
    int localeNameLen = 1;

    // Dynamically load GetUserDefaultLocaleName function, as it is not available on XP.
    SkGetUserDefaultLocaleNameProc getUserDefaultLocaleNameProc = nullptr;
    HRESULT hr = SkGetGetUserDefaultLocaleNameProc(&getUserDefaultLocaleNameProc);
    if (nullptr == getUserDefaultLocaleNameProc) {
        SK_TRACEHR(hr, "Could not get GetUserDefaultLocaleName.");
    } else {
        int size = getUserDefaultLocaleNameProc(localeNameStorage, LOCALE_NAME_MAX_LENGTH);
        if (size) {
            localeName = localeNameStorage;
            localeNameLen = size;
        }
    }

    return sk_sp(new SkNativeFontCollection_DirectWrite_impl(factory, collection, fallback,
                                             localeName, localeNameLen,
                                             defaultFamilyName, defaultFamilyNameLen));
}

SkNativeFontCollection_DirectWrite::Match::Match(Match&& that) : fFamily(that.fFamily), fFont(that.fFont), fFace(that.fFace) {
    that.fFamily = nullptr;
    that.fFont = nullptr;
    that.fFace = nullptr;
}
SkNativeFontCollection_DirectWrite::Match& SkNativeFontCollection_DirectWrite::Match::operator=(Match&& that) {
    this->fFamily = that.fFamily; that.fFamily = nullptr;
    this->fFont = that.fFont; that.fFont = nullptr;
    this->fFace = that.fFace; that.fFace = nullptr;
    return *this;
}
SkNativeFontCollection_DirectWrite::Match::~Match() {
    if (fFamily) { fFamily->Release(); }
    if (fFont) { fFont->Release(); }
    if (fFace) { fFace->Release(); }
};

#endif//defined(SK_BUILD_FOR_WIN)
