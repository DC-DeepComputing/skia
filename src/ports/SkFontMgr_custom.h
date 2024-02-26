/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TODO: How to get that in here.
#define SK_TYPEFACE_FACTORY_FONTATIONS 1


#ifndef SkFontMgr_custom_DEFINED
#define SkFontMgr_custom_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "src/ports/SkTypeface_Fontations_priv.h"
#else
#include "src/ports/SkTypeface_FreeType.h"
#endif

class SkData;
class SkFontDescriptor;
class SkStreamAsset;
class SkTypeface;
class SkFontScanner;


/** The base SkTypeface implementation for the custom font manager. */
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
class SkTypeface_Custom : public SkTypeface_Fontations {
#else
class SkTypeface_Custom : public SkTypeface_FreeType {
#endif
public:
    SkTypeface_Custom(const SkFontStyle& style, bool isFixedPitch,
                      bool sysFont, SkString familyName, int index);
    bool isSysFont() const;

protected:
    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const override;
    int getIndex() const;

private:
    const bool fIsSysFont;
    const SkString fFamilyName;
    const int fIndex;

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
    using INHERITED = SkTypeface_Fontations;
#else
    using INHERITED = SkTypeface_FreeType;
#endif
};

/** The empty SkTypeface implementation for the custom font manager.
 *  Used as the last resort fallback typeface.
 */
class SkTypeface_Empty : public SkTypeface_Custom {
public:
    SkTypeface_Empty() ;

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int*) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
#if !defined(SK_TYPEFACE_FACTORY_FONTATIONS)
    std::unique_ptr<SkFontData> onMakeFontData() const override;
#endif

private:
    using INHERITED = SkTypeface_Custom;
};

/** The file SkTypeface implementation for the custom font manager. */
class SkTypeface_File : public SkTypeface_Custom {
public:
    SkTypeface_File(const SkFontStyle& style, bool isFixedPitch, bool sysFont,
                    SkString familyName, const char path[], int index);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
#if !defined(SK_TYPEFACE_FACTORY_FONTATIONS)
    std::unique_ptr<SkFontData> onMakeFontData() const override;
#endif

private:
    SkString fPath;

    using INHERITED = SkTypeface_Custom;
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  SkFontStyleSet_Custom
 *
 *  This class is used by SkFontMgr_Custom to hold SkTypeface_Custom families.
 */
class SkFontStyleSet_Custom : public SkFontStyleSet {
public:
    explicit SkFontStyleSet_Custom(SkString familyName);

    /** Should only be called during the initial build phase. */
    void appendTypeface(sk_sp<SkTypeface> typeface);
    int count() override;
    void getStyle(int index, SkFontStyle* style, SkString* name) override;
    sk_sp<SkTypeface> createTypeface(int index) override;
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override;
    SkString getFamilyName();

private:
    skia_private::TArray<sk_sp<SkTypeface>> fStyles;
    SkString fFamilyName;

    friend class SkFontMgr_Custom;
};

/**
 *  SkFontMgr_Custom
 *
 *  This class is essentially a collection of SkFontStyleSet_Custom,
 *  one SkFontStyleSet_Custom for each family. This class may be modified
 *  to load fonts from any source by changing the initialization.
 */
class SkFontMgr_Custom : public SkFontMgr {
public:
    typedef skia_private::TArray<sk_sp<SkFontStyleSet_Custom>> Families;
    class SystemFontLoader {
    public:
        virtual ~SystemFontLoader() { }
        virtual void loadSystemFonts(const SkFontScanner*, Families*) const = 0;
    };
    explicit SkFontMgr_Custom(
            const SystemFontLoader& loader,
            SkTypeface::FactoryId factoryId = SkSetFourByteTag('f', 'r', 'e', 'e'));

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontStyle) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                  const char* bcp47[], int bcp47Count,
                                                  SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override;

private:
    Families fFamilies;
    sk_sp<SkFontStyleSet> fDefaultFamily;
    std::unique_ptr<SkFontScanner> fScanner;
    SkTypeface::FactoryId fFactoryId;
};

#endif
