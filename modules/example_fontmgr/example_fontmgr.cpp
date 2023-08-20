/*
 * Copyright 2023 Pentrek Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"

#include "src/ports/SkTypeface_mac_ct.h"

namespace {

sk_sp<SkTypeface> make_from_stream(std::unique_ptr<SkStreamAsset> stream,
                                   const SkFontArguments& args) {
    return SkTypeface_Mac::MakeFromStream(std::move(stream), args);
}

class ExampleFontStyleSet : public SkFontStyleSet {
    sk_sp<SkTypeface> fTF;

public:
    ExampleFontStyleSet(sk_sp<SkTypeface> tf) : fTF(std::move(tf)) {}

    int count() override {
        return 1;
    }
    void getStyle(int index, SkFontStyle* style, SkString* styleName) override {
        assert(index == 0);
        if (style) {
            *style = fTF->fontStyle();
        }
        if (styleName) {
            styleName->set("Regular");
        }
    }
    sk_sp<SkTypeface> createTypeface(int index) override {
        assert(index == 0);
        return fTF;
    }
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override {
        // do we return null if not a complete match, or best effort?
        return fTF;
    }
};


class ExampleFontMgr : public SkFontMgr {
    sk_sp<SkTypeface> fTF;

    sk_sp<SkTypeface> nullIfMismatch(const char familyName[]) const {
        if (familyName) {
            SkString name;
            fTF->getFamilyName(&name);
            if (!name.equals(familyName)) {
                return nullptr;
            }
        }
        return fTF;
    }

public:
    ExampleFontMgr(sk_sp<SkTypeface> def) : fTF(std::move(def)) {}

protected:
    int onCountFamilies() const override { return 1; }

    void onGetFamilyName(int index, SkString* familyName) const override {
        assert(index == 0);
        if (familyName) {
            fTF->getFamilyName(familyName);
        }
    }

    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        assert(index == 0);
        return sk_make_sp<ExampleFontStyleSet>(fTF);
    }

    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        if (auto tf = this->nullIfMismatch(familyName)) {
            return sk_make_sp<ExampleFontStyleSet>(tf);
        }
        return nullptr;
    }

    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        return this->nullIfMismatch(familyName);    // check style?
    }

    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        return this->nullIfMismatch(familyName);    // check character?
    }

    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        return this->makeFromStream(std::unique_ptr<SkStreamAsset>(new SkMemoryStream(std::move(data))),
                                    ttcIndex);
    }

    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path);
        return stream ? this->makeFromStream(std::move(stream), ttcIndex) : nullptr;
    }

    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        return this->makeFromStream(std::move(stream),
                                    SkFontArguments().setCollectionIndex(ttcIndex));
    }

    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        return make_from_stream(std::move(stream), args);
    }

    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle style) const override {
        return this->matchFamilyStyle(familyName, style);
    }

private:
    using INHERITED = SkFontMgr;
};

} // namespace

////////////////////

sk_sp<SkFontMgr> SkFontMgr::Factory() {
    auto st = SkStream::MakeFromFile("/Users/mike/skia/trunk/resources/fonts/Roboto-Regular.ttf");
    if (!st) {
        printf("-- failed to open font file\n");
        return nullptr;
    }

    auto tf = make_from_stream(std::move(st), SkFontArguments());
    if (!tf) {
        printf("failed to create typeface from stream\n");
        return nullptr;
    }

    printf("Making example fontmgr\n");
    return sk_make_sp<ExampleFontMgr>(std::move(tf));
}


