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

#include "include/ports/SkTypeface_win.h"

#include <dwrite.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

using namespace skia_private;

class SkFontMgr_DirectWrite : public SkFontMgr {
public:
    /** localeNameLength and defaultFamilyNameLength must include the null terminator. */
    SkFontMgr_DirectWrite(sk_sp<SkNativeFontCollection_DirectWrite> native)
        : fNative(std::move(native))
    {}

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontstyle) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                  const char* bcp47[], int bcp47Count,
                                                  SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override;

private:
    /** Creates a typeface using a typeface cache. */
    sk_sp<SkTypeface> makeTypefaceFromDWriteFont(SkNativeFontCollection_DirectWrite::Match) const;

    sk_sp<SkNativeFontCollection_DirectWrite> fNative;
    mutable SkMutex fTFCacheMutex;
    mutable SkTypefaceCache fTFCache;

    friend class SkFontStyleSet_DirectWrite;
};

class SkFontStyleSet_DirectWrite : public SkFontStyleSet {
public:
    SkFontStyleSet_DirectWrite(const SkFontMgr_DirectWrite* fontMgr,
                               IDWriteFontFamily* fontFamily)
        : fFontMgr(SkRef(fontMgr))
        , fFontFamily(SkRefComPtr(fontFamily))
    { }

    int count() override;
    void getStyle(int index, SkFontStyle* fs, SkString* styleName) override;
    sk_sp<SkTypeface> createTypeface(int index) override;
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override;

private:
    sk_sp<const SkFontMgr_DirectWrite> fFontMgr;
    SkTScopedComPtr<IDWriteFontFamily> fFontFamily;
};

static HRESULT are_same(IUnknown* a, IUnknown* b, bool& same) {
    SkTScopedComPtr<IUnknown> iunkA;
    HRM(a->QueryInterface(&iunkA), "Failed to QI<IUnknown> for a.");

    SkTScopedComPtr<IUnknown> iunkB;
    HRM(b->QueryInterface(&iunkB), "Failed to QI<IUnknown> for b.");

    same = (iunkA.get() == iunkB.get());
    return S_OK;
}

struct ProtoDWriteTypeface {
    IDWriteFontFace* fDWriteFontFace;
    IDWriteFont* fDWriteFont;
    IDWriteFontFamily* fDWriteFontFamily;
};

static bool FindByDWriteFont(SkTypeface* cached, void* ctx) {
    DWriteFontTypeface* cshFace = reinterpret_cast<DWriteFontTypeface*>(cached);
    ProtoDWriteTypeface* ctxFace = reinterpret_cast<ProtoDWriteTypeface*>(ctx);

    // IDWriteFontFace5 introduced both Equals and HasVariations
    SkTScopedComPtr<IDWriteFontFace5> cshFontFace5;
    SkTScopedComPtr<IDWriteFontFace5> ctxFontFace5;
    cshFace->fDWriteFontFace->QueryInterface(&cshFontFace5);
    ctxFace->fDWriteFontFace->QueryInterface(&ctxFontFace5);
    if (cshFontFace5 && ctxFontFace5) {
        return cshFontFace5->Equals(ctxFontFace5.get());
    }

    bool same;

    //Check to see if the two fonts are identical.
    HRB(are_same(cshFace->fDWriteFont.get(), ctxFace->fDWriteFont, same));
    if (same) {
        return true;
    }

    HRB(are_same(cshFace->fDWriteFontFace.get(), ctxFace->fDWriteFontFace, same));
    if (same) {
        return true;
    }

    //Check if the two fonts share the same loader and have the same key.
    UINT32 cshNumFiles;
    UINT32 ctxNumFiles;
    HRB(cshFace->fDWriteFontFace->GetFiles(&cshNumFiles, nullptr));
    HRB(ctxFace->fDWriteFontFace->GetFiles(&ctxNumFiles, nullptr));
    if (cshNumFiles != ctxNumFiles) {
        return false;
    }

    SkTScopedComPtr<IDWriteFontFile> cshFontFile;
    SkTScopedComPtr<IDWriteFontFile> ctxFontFile;
    HRB(cshFace->fDWriteFontFace->GetFiles(&cshNumFiles, &cshFontFile));
    HRB(ctxFace->fDWriteFontFace->GetFiles(&ctxNumFiles, &ctxFontFile));

    //for (each file) { //we currently only admit fonts from one file.
    SkTScopedComPtr<IDWriteFontFileLoader> cshFontFileLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> ctxFontFileLoader;
    HRB(cshFontFile->GetLoader(&cshFontFileLoader));
    HRB(ctxFontFile->GetLoader(&ctxFontFileLoader));
    HRB(are_same(cshFontFileLoader.get(), ctxFontFileLoader.get(), same));
    if (!same) {
        return false;
    }
    //}

    const void* cshRefKey;
    UINT32 cshRefKeySize;
    const void* ctxRefKey;
    UINT32 ctxRefKeySize;
    HRB(cshFontFile->GetReferenceKey(&cshRefKey, &cshRefKeySize));
    HRB(ctxFontFile->GetReferenceKey(&ctxRefKey, &ctxRefKeySize));
    if (cshRefKeySize != ctxRefKeySize) {
        return false;
    }
    if (0 != memcmp(cshRefKey, ctxRefKey, ctxRefKeySize)) {
        return false;
    }

    //TODO: better means than comparing name strings?
    //NOTE: .ttc and fake bold/italic will end up here.
    SkTScopedComPtr<IDWriteLocalizedStrings> cshFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> cshFaceNames;
    HRB(cshFace->fDWriteFontFamily->GetFamilyNames(&cshFamilyNames));
    HRB(cshFace->fDWriteFont->GetFaceNames(&cshFaceNames));
    UINT32 cshFamilyNameLength;
    UINT32 cshFaceNameLength;
    HRB(cshFamilyNames->GetStringLength(0, &cshFamilyNameLength));
    HRB(cshFaceNames->GetStringLength(0, &cshFaceNameLength));

    SkTScopedComPtr<IDWriteLocalizedStrings> ctxFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> ctxFaceNames;
    HRB(ctxFace->fDWriteFontFamily->GetFamilyNames(&ctxFamilyNames));
    HRB(ctxFace->fDWriteFont->GetFaceNames(&ctxFaceNames));
    UINT32 ctxFamilyNameLength;
    UINT32 ctxFaceNameLength;
    HRB(ctxFamilyNames->GetStringLength(0, &ctxFamilyNameLength));
    HRB(ctxFaceNames->GetStringLength(0, &ctxFaceNameLength));

    if (cshFamilyNameLength != ctxFamilyNameLength ||
        cshFaceNameLength != ctxFaceNameLength)
    {
        return false;
    }

    SkSMallocWCHAR cshFamilyName(cshFamilyNameLength+1);
    SkSMallocWCHAR cshFaceName(cshFaceNameLength+1);
    HRB(cshFamilyNames->GetString(0, cshFamilyName.get(), cshFamilyNameLength+1));
    HRB(cshFaceNames->GetString(0, cshFaceName.get(), cshFaceNameLength+1));

    SkSMallocWCHAR ctxFamilyName(ctxFamilyNameLength+1);
    SkSMallocWCHAR ctxFaceName(ctxFaceNameLength+1);
    HRB(ctxFamilyNames->GetString(0, ctxFamilyName.get(), ctxFamilyNameLength+1));
    HRB(ctxFaceNames->GetString(0, ctxFaceName.get(), ctxFaceNameLength+1));

    return wcscmp(cshFamilyName.get(), ctxFamilyName.get()) == 0 &&
           wcscmp(cshFaceName.get(), ctxFaceName.get()) == 0;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::makeTypefaceFromDWriteFont(
        SkNativeFontCollection_DirectWrite::Match match) const {
    if (!match) {
        return nullptr;
    }
    SkAutoMutexExclusive ama(fTFCacheMutex);
    ProtoDWriteTypeface spec = { match.fFace, match.fFont, match.fFamily };
    sk_sp<SkTypeface> face = fTFCache.findByProcAndRef(FindByDWriteFont, &spec);
    if (nullptr == face) {
        face = DWriteFontTypeface::Make(fNative->factory(), match.fFace, match.fFont, match.fFamily, nullptr,
                                        SkFontArguments::Palette{0, nullptr, 0});
        if (face) {
            fTFCache.add(face);
        }
    }
    return face;
}

int SkFontMgr_DirectWrite::onCountFamilies() const {
    return fNative->countFamilies();
}

void SkFontMgr_DirectWrite::onGetFamilyName(int index, SkString* familyName) const {
    fNative->getFamilyName(index, familyName);
}

sk_sp<SkFontStyleSet> SkFontMgr_DirectWrite::onCreateStyleSet(int index) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily(fNative->createStyleSet(index));
    if (!fontFamily) {
        return nullptr;
    }

    return sk_sp<SkFontStyleSet>(new SkFontStyleSet_DirectWrite(this, fontFamily.get()));
}

sk_sp<SkFontStyleSet> SkFontMgr_DirectWrite::onMatchFamily(const char familyName[]) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily(fNative->matchFamily(familyName));

    return sk_sp<SkFontStyleSet>(new SkFontStyleSet_DirectWrite(this, fontFamily.get()));
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMatchFamilyStyle(const char familyName[],
                                                            const SkFontStyle& fontstyle) const {
    SkNativeFontCollection_DirectWrite::Match match(fNative->matchFamilyStyle(familyName, fontstyle));
    return this->makeTypefaceFromDWriteFont(std::move(match));
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMatchFamilyStyleCharacter(
    const char familyName[], const SkFontStyle& style,
    const char* bcp47[], int bcp47Count,
    SkUnichar character) const
{
    DWriteStyle dwStyle(style);

    SkNativeFontCollection_DirectWrite::Match match(fNative->matchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, character));
    return this->makeTypefaceFromDWriteFont(std::move(match));
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                               int ttcIndex) const {
    SkFontArguments args;
    args.setCollectionIndex(ttcIndex);
    return this->onMakeFromStreamArgs(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                              const SkFontArguments& args) const {
    return DWriteFontTypeface::MakeFromStream(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromFile(const char path[], int ttcIndex) const {
    return this->makeFromStream(SkStream::MakeFromFile(path), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onLegacyMakeTypeface(const char familyName[],
                                                              SkFontStyle style) const {
    SkNativeFontCollection_DirectWrite::Match match(fNative->legacyMakeTypeface(familyName, style));
    return this->makeTypefaceFromDWriteFont(std::move(match));
}

///////////////////////////////////////////////////////////////////////////////

int SkFontStyleSet_DirectWrite::count() {
    return fFontFamily->GetFontCount();
}

sk_sp<SkTypeface> SkFontStyleSet_DirectWrite::createTypeface(int index) {
    SkTScopedComPtr<IDWriteFont> font;
    HRNM(fFontFamily->GetFont(index, &font), "Could not get font.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->makeTypefaceFromDWriteFont(SkNativeFontCollection_DirectWrite::Match(fFontFamily.release(), font.release(), fontFace.release()));
}

void SkFontStyleSet_DirectWrite::getStyle(int index, SkFontStyle* fs, SkString* styleName) {
    SkTScopedComPtr<IDWriteFont> font;
    HRVM(fFontFamily->GetFont(index, &font), "Could not get font.");

    if (fs) {
        SkTScopedComPtr<IDWriteFontFace> face;
        HRVM(font->CreateFontFace(&face), "Could not get face.");
        *fs = DWriteFontTypeface::GetStyle(font.get(), face.get());
    }

    if (styleName) {
        SkTScopedComPtr<IDWriteLocalizedStrings> faceNames;
        if (SUCCEEDED(font->GetFaceNames(&faceNames))) {
            sk_get_locale_string(faceNames.get(), fFontMgr->fNative->locale(), styleName);
        }
    }
}

sk_sp<SkTypeface> SkFontStyleSet_DirectWrite::matchStyle(const SkFontStyle& pattern) {
    SkNativeFontCollection_DirectWrite::Match match (fFontMgr->fNative->matchFamilyStyle(fFontFamily.get(), pattern));
    if (!match) {
        return nullptr;
    }
    return fFontMgr->makeTypefaceFromDWriteFont(std::move(match));
}

////////////////////////////////////////////////////////////////////////////////
#include "include/ports/SkTypeface_win.h"

sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                           IDWriteFontCollection* collection) {
    return SkFontMgr_New_DirectWrite(factory, collection, nullptr);
}

sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                           IDWriteFontCollection* collection,
                                           IDWriteFontFallback* fallback) {
    return sk_sp(new SkFontMgr_DirectWrite(SkNativeFontCollection_DirectWrite::Make(factory, collection, fallback)));
}

#endif//defined(SK_BUILD_FOR_WIN)
