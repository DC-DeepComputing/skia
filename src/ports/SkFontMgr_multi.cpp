/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTArray.h"
#include "include/ports/SkFontMgr_multi.h"
#include "src/core/SkZip.h"

class SkMultiFontMgr : public SkFontMgr {
public:
    SkMultiFontMgr(SkSpan<const sk_sp<SkFontMgr>> fontmgrs)
        : fFontMgrs(fontmgrs.data(), fontmgrs.size())
    {
        int end = 0;
        for (auto&& fontmgr : fFontMgrs) {
            end += fontmgr->countFamilies();
            fEndIndex.push_back(end);
        }
    }

protected:
    int onCountFamilies() const override {
        return fEndIndex.empty() ? 0 : fEndIndex.back();
    }

    void onGetFamilyName(int index, SkString* familyName) const override {
        int startIndex = 0;
        for (auto&& [fontmgr, endIndex] : SkMakeZip(fFontMgrs, fEndIndex)) {
            if (index < endIndex) {
                fontmgr->getFamilyName(index - startIndex, familyName);
                return;
            }
            startIndex = endIndex;
        }
        SkDEBUGFAIL("onGetFamilyName called with bad index");
        return;
    }

    SkFontStyleSet* onCreateStyleSet(int index) const override {
        int startIndex = 0;
        for (auto&& [fontmgr, endIndex] : SkMakeZip(fFontMgrs, fEndIndex)) {
            if (index < endIndex) {
                return fontmgr->createStyleSet(index - startIndex);
            }
            startIndex = endIndex;
        }
        SkDEBUGFAIL("onCreateStyleSet called with bad index");
        return nullptr;
    }

    SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkFontStyleSet> styleSet(fontmgr->matchFamily(familyName));
            if (styleSet->count() > 0) {
                return styleSet.release();
            }
        }
        return nullptr;
    }

    SkTypeface* onMatchFamilyStyle(const char familyName[], const SkFontStyle& style) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->matchFamilyStyle(familyName, style));
            if (typeface) {
                return typeface.release();
            }
        }
        return nullptr;
    }

    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                            const SkFontStyle& style,
                                            const char* bcp47[],
                                            int bcp47Count,
                                            SkUnichar character) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(
                fontmgr->matchFamilyStyleCharacter(familyName,style, bcp47, bcp47Count, character));
            if (typeface && typeface->unicharToGlyph(character) != 0) {
                return typeface.release();
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->makeFromData(data, ttcIndex));
            if (typeface) {
                return typeface;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->makeFromStream(std::move(stream), ttcIndex));
            if (typeface) {
                return typeface;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->makeFromStream(std::move(stream), args));
            if (typeface) {
                return typeface;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMakeFromFile(const char filePath[], int ttcIndex) const override {
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->makeFromFile(filePath, ttcIndex));
            if (typeface) {
                return typeface;
            }
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override {
        size_t familyNameSize = strlen(familyName);
        sk_sp<SkTypeface> first;
        for (auto&& fontmgr : fFontMgrs) {
            sk_sp<SkTypeface> typeface(fontmgr->legacyMakeTypeface(familyName, style));
            if (typeface) {
                sk_sp<SkTypeface::LocalizedStrings> names(typeface->createFamilyNameIterator());
                SkTypeface::LocalizedString name;
                while (names->next(&name)) {
                    if (name.fString.equals(familyName, familyNameSize)) {
                        return typeface;
                    }
                }
                if (!first || (first->countGlyphs() == 0 && typeface->countGlyphs() > 0)) {
                    first = typeface;
                }
            }
        }
        return first;
    }

private:
    SkSTArray<4, sk_sp<SkFontMgr>> fFontMgrs;
    SkSTArray<4, int> fEndIndex;
};

sk_sp<SkFontMgr> SkFontMgr_New_Multi(SkSpan<const sk_sp<SkFontMgr>> fontmgrs) {
    return sk_make_sp<SkMultiFontMgr>(fontmgrs);
}
