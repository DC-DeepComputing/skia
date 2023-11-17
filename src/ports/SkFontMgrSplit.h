/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgrSplit_DEFINED
#define SkFontMgrSplit_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/core/SkTypeface.h"
#include <memory>

// Font matching interface (system-dependent)
struct FontMatch {
    typedef enum System {
        kUndefined = 0,
        kFontConfig = 1,
        kDWriteFontFace = 2,
    } ResultsType;
    FontMatch(ResultsType rt) : fResultsType(rt) { }
    ResultsType fResultsType;
};

struct FontConfigMatch : public FontMatch {
    FontConfigMatch()
            : FontMatch(ResultsType::kFontConfig) { }
    // Raw font data required to instantiate a font
};

struct DWriteFontFaceMatch : public FontMatch {
    DWriteFontFaceMatch()
            : FontMatch(ResultsType::kDWriteFontFace) { }
    // Raw font data required to instantiate a font
};

class FontMatcher {
 public:
    template<typename T> static T matchFamilyStyle(const char familyName[],
                                                   const SkFontStyle& fontStyle);
    template<typename T> static T matchFamilyStyleCharacter(const char familyName[],
                                                            const SkFontStyle&,
                                                            const char* bcp47[], int bcp47Count,
                                                            SkUnichar character);
};

// Instantiating interface (platform-dependent)
class FontFactory {
    public:
        sk_sp<SkTypeface> makeTypeface(const FontMatch& results) {
            switch (results.fResultsType) {
                case FontMatch::ResultsType::kFontConfig:
                    return this->makeTypeface(static_cast<const FontConfigMatch&>(results));
                case FontMatch::ResultsType::kDWriteFontFace:
                    return this->makeTypeface(static_cast<const DWriteFontFaceMatch&>(results));
                default:
                    return nullptr;
            }
        }

        // Since we cannot make a virtual method templated we need to list them all here:
        virtual sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& results) {
            // Should we say "not implemented"?
            return nullptr;
        };
        virtual sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& results) {
            return nullptr;
        };
};

class FreeTypeFontFactory : public FontFactory {
    public:
        sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& results) override;
        sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& results) override;
    private:
    // Some data, cache and whatever
};

class FontationsFontFactory : public FontFactory {
    public:
        sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& results) override;
        sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& results) override;
    private:
    // Some data, cache and whatever
};

void test() {
    FreeTypeFontFactory* freeTypeFontFactory;
    FontationsFontFactory* fontationsFontFactory;

    // Specific platform
    auto matchResults =
            FontMatcher::matchFamilyStyle<FontConfigMatch>("Roboto", SkFontStyle::BoldItalic());
    auto freeTypeTypeface = freeTypeFontFactory->makeTypeface(matchResults);
    auto fontationsTypeface = fontationsFontFactory->makeTypeface(matchResults);

    // Any system, any platform
    FontFactory* anyFactory;
    auto anyMatchResults = static_cast<FontMatch>(matchResults);
    auto anyTypeface = anyFactory->makeTypeface(anyMatchResults);
}

#endif // SkFontMgrSplit_DEFINED