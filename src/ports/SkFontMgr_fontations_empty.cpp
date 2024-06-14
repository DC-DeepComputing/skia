/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_Fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"

namespace {
/**
 *  SkFontMgr_Fontations_Empty
 *
 *  A SkFontMgr with an empty list of fonts, meant as a basic tool for font instantiation from
 *  data in testing, see TestFontMgr in FontToolUtils.
 */
class SkFontMgr_Fontations_Empty : public SkFontMgr {
public:
    SkFontMgr_Fontations_Empty() = default;

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontStyle) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle&,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                            int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                           const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override;
};

int SkFontMgr_Fontations_Empty::onCountFamilies() const { return 0; }
void SkFontMgr_Fontations_Empty::onGetFamilyName(int index, SkString* familyName) const {}

sk_sp<SkFontStyleSet> SkFontMgr_Fontations_Empty::onCreateStyleSet(int index) const {
    return SkFontStyleSet::CreateEmpty();
}
sk_sp<SkFontStyleSet> SkFontMgr_Fontations_Empty::onMatchFamily(const char familyName[]) const {
    return SkFontStyleSet::CreateEmpty();
}
sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMatchFamilyStyle(
        const char familyName[], const SkFontStyle& fontStyle) const {
    return nullptr;
}
sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMatchFamilyStyleCharacter(
        const char familyName[],
        const SkFontStyle&,
        const char* bcp47[],
        int bcp47Count,
        SkUnichar character) const {
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMakeFromData(sk_sp<SkData> data,
                                                             int ttcIndex) const {
    return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMakeFromStreamIndex(
        std::unique_ptr<SkStreamAsset> stream, int ttcIndex) const {
    return this->makeFromStream(std::move(stream), SkFontArguments().setCollectionIndex(ttcIndex));
}

sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMakeFromStreamArgs(
        std::unique_ptr<SkStreamAsset> stream, const SkFontArguments& args) const {
    return SkTypeface_Fontations::MakeFromStream(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onMakeFromFile(const char path[],
                                                             int ttcIndex) const {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
    return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Fontations_Empty::onLegacyMakeTypeface(const char familyName[],
                                                                   SkFontStyle style) const {
    return nullptr;
}

}  // namespace

sk_sp<SkFontMgr> SkFontMgr_New_Fontations_Empty() {
    return sk_make_sp<SkFontMgr_Fontations_Empty>();
}
