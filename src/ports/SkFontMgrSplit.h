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

namespace test1 {
struct FontMatch{ };

struct FontConfigMatch : public FontMatch {
// Raw data required for font instantiation
};

struct DWriteFontFaceMatch : public FontMatch {
// Raw data required for font instantiation
};

class FontManager {
   public:
      // Matching functionality
      template<typename T> static T matchFamilyStyle(...);
      template<typename T> static T matchFamilyStyleCharacter(...);

      // Instantiating functionality
      virtual sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& match) = 0;
      virtual sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& match) = 0;
};

class FreeTypeFontManager : public FontManager {
    public:
        sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& match) override { /* ... */ }
        sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& match) override { /* ... */ }
    private:
        // Cache and all the other data
};

class FontationsFontManager : public FontManager {
    public:
        sk_sp<SkTypeface> makeTypeface(const FontConfigMatch& match) override { /* ... */ }
        sk_sp<SkTypeface> makeTypeface(const DWriteFontFaceMatch& match) override { /* ... */ }
    private:
        // Cache and all the other backend-specific data
};

void test1() {
    FreeTypeFontManager* freeTypeFontManager;
    FontationsFontManager* fontationsFontManager;
    // ...
    auto fontConfigMatch = FontManager::matchFamilyStyle<FontConfigMatch>();
    auto freeTypeTypeface = freeTypeFontManager->makeTypeface(fontConfigMatch);
    auto fontationsTypeface = fontationsFontManager->makeTypeface(fontConfigMatch);
}
}

namespace test2 {
class FontMatcher {
 public:
    template<typename T> static T matchFamilyStyle(...);
    template<typename T> static T matchFamilyStyleCharacter(...);
};

struct FontMatchingResults {
    typedef enum ResultsType {
        kUndefined = 0,
        kFontConfig = 1,
        kFontations = 2,
    } ResultsType;
    FontMatchingResults(ResultsType rt) : fResultsType(rt) { }
    ResultsType fResultsType;
};

struct FontConfigMatchingResults : public FontMatchingResults {
    FontConfigMatchingResults()
        : FontMatchingResults(ResultsType::kFontConfig) { }
    // Raw font data required to instantiate a font
};

struct FontationsMatchingResults : public FontMatchingResults {
    FontationsMatchingResults()
        : FontMatchingResults(ResultsType::kFontations) { }
    // Raw font data required to instantiate a font
};

// Instantiating interface
class FontFactory {
    public:
        sk_sp<SkTypeface> Instantiate(const FontMatchingResults& results) {
            switch (results.fResultsType) {
                case FontMatchingResults::ResultsType::kFontConfig:
                    return instantiateFontConfigTypeface(static_cast<const FontConfigMatchingResults&>(results));
                case FontMatchingResults::ResultsType::kFontations:
                    return instantiateFontationsTypeface(static_cast<const FontationsMatchingResults&>(results));
                default:
                    return nullptr;
            }
        }

        virtual sk_sp<SkTypeface> instantiateFontConfigTypeface(const FontConfigMatchingResults& results) {
            return nullptr;
        };
        virtual sk_sp<SkTypeface> instantiateFontationsTypeface(const FontationsMatchingResults& results) {
            return nullptr;
        };
};

class FreeTypeFontFactory : public FontFactory {
    public:
        sk_sp<SkTypeface> instantiateFontConfigTypeface(const FontConfigMatchingResults& results);
        sk_sp<SkTypeface> instantiateFontationsTypeface(const FontationsMatchingResults& results);
    private:
    // Some data, cache and whatever
};

class FontationsFontFactory : public FontFactory {
    public:
        sk_sp<SkTypeface> instantiateFontConfigTypeface(const FontConfigMatchingResults& results);
        sk_sp<SkTypeface> instantiateFontationsTypeface(const FontationsMatchingResults& results);
    private:
    // Some data, cache and whatever
};

void test2() {
    FreeTypeFontFactory* freeTypeFontFactory;
    FontationsFontFactory* fontationsFontFactory;
    //
    auto matchResults = FontMatcher::matchFamilyStyle<FontConfigMatchingResults>();
    auto freeTypeTypeface = freeTypeFontFactory->Instantiate(matchResults);
    auto fontationsTypeface = fontationsFontFactory->Instantiate(matchResults);

}
}

#endif // SkFontMgrSplit_DEFINED