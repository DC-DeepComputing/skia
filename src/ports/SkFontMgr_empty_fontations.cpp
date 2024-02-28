/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkFontMgr_empty_fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"

int SkFontMgr_Empty_Fontations::onCountFamilies() const { return 0; }
void SkFontMgr_Empty_Fontations::onGetFamilyName(int index, SkString* familyName) const {}

sk_sp<SkFontStyleSet> SkFontMgr_Empty_Fontations::onCreateStyleSet(int index) const {
    return SkFontStyleSet::CreateEmpty();
}
sk_sp<SkFontStyleSet> SkFontMgr_Empty_Fontations::onMatchFamily(const char familyName[]) const {
    return SkFontStyleSet::CreateEmpty();
}
sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMatchFamilyStyle(
        const char familyName[], const SkFontStyle& fontStyle) const {
    return nullptr;
}
sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMatchFamilyStyleCharacter(
        const char familyName[],
        const SkFontStyle&,
        const char* bcp47[],
        int bcp47Count,
        SkUnichar character) const {
    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMakeFromData(sk_sp<SkData> data,
                                                             int ttcIndex) const {
    return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMakeFromStreamIndex(
        std::unique_ptr<SkStreamAsset> stream, int ttcIndex) const {
    return this->makeFromStream(std::move(stream), SkFontArguments().setCollectionIndex(ttcIndex));
}

sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMakeFromStreamArgs(
        std::unique_ptr<SkStreamAsset> stream, const SkFontArguments& args) const {
    return SkTypeface_Fontations::MakeFromStream(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onMakeFromFile(const char path[],
                                                             int ttcIndex) const {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
    return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
}

sk_sp<SkTypeface> SkFontMgr_Empty_Fontations::onLegacyMakeTypeface(const char familyName[],
                                                                   SkFontStyle style) const {
    return nullptr;
}

sk_sp<SkFontMgr> SkFontMgr_New_Empty_Fontations() {
    return sk_make_sp<SkFontMgr_Empty_Fontations>();
}
