/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#if defined(SK_BUILD_FOR_UNIX) && defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "src/ports/SkFontMgr_fontconfig_priv.h"
#include "tools/fonts/FontToolUtils.h"

#include <fontconfig/fontconfig.h>
#include <memory>

namespace {

void add_pattern_string(FcPattern* pattern, const char* object, const char* value, bool strong) {
    FcValue val;
    val.type = FcTypeString;
    val.u.s = reinterpret_cast<const FcChar8*>(value);
    if (strong) {
        FcPatternAdd(pattern, object, val, FcTrue);
    } else {
        FcPatternAddWeak(pattern, object, val, FcTrue);
    }
}

struct FcPatternDeleter {
    void operator()(FcPattern* p) const {
        if (p) {
            FcPatternDestroy(p);
        }
    }
};

using UniqueFcPattern = std::unique_ptr<FcPattern, FcPatternDeleter>;

}  // namespace

// Benchmark for SkFontMgr_fontconfig_priv::remove_weak
class FontConfigRemoveWeakBench : public Benchmark {
public:
    enum class Scenario {
        kSansSubstitute,  // 6 weak pre, 1 strong, 8 weak post (typical 'sans-serif' substituted
                          // pattern)
        kManyElements,    // 25 elements (1 strong, 24 weak)
        kFewElements,     // 2 elements (1 strong, 1 weak)
        kAllStrong,       // 5 strong elements (no removal)
        kAllWeak,         // 5 weak elements (all weak early-exit)
    };

    FontConfigRemoveWeakBench(Scenario scenario, const char* name)
            : fScenario(scenario), fName(SkStringPrintf("fontconfig_remove_weak_%s", name)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        fTemplate.reset(FcPatternCreate());
        switch (fScenario) {
            case Scenario::kSansSubstitute:
                // Pre-strong weak elements (e.g. preferred aliases)
                for (int i = 0; i < 6; ++i) {
                    add_pattern_string(fTemplate.get(), FC_FAMILY, "WeakPreFamily", false);
                }
                // Strong element
                add_pattern_string(fTemplate.get(), FC_FAMILY, "StrongFamily", true);
                // Trailing weak elements (default fallback families to be stripped)
                for (int i = 0; i < 8; ++i) {
                    add_pattern_string(fTemplate.get(), FC_FAMILY, "WeakPostFamily", false);
                }
                break;
            case Scenario::kManyElements:
                for (int i = 0; i < 25; ++i) {
                    if (i == 5) {
                        add_pattern_string(fTemplate.get(), FC_FAMILY, "StrongFamily", true);
                    } else {
                        add_pattern_string(fTemplate.get(), FC_FAMILY, "WeakFamily", false);
                    }
                }
                break;
            case Scenario::kFewElements:
                add_pattern_string(fTemplate.get(), FC_FAMILY, "StrongFamily", true);
                add_pattern_string(fTemplate.get(), FC_FAMILY, "WeakFamily", false);
                break;
            case Scenario::kAllStrong:
                for (int i = 0; i < 5; ++i) {
                    add_pattern_string(fTemplate.get(), FC_FAMILY, "StrongFamily", true);
                }
                break;
            case Scenario::kAllWeak:
                for (int i = 0; i < 5; ++i) {
                    add_pattern_string(fTemplate.get(), FC_FAMILY, "WeakFamily", false);
                }
                break;
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            FcPattern* pat = FcPatternDuplicate(fTemplate.get());
            SkFontMgr_fontconfig_priv::remove_weak(pat, FC_FAMILY);
            FcPatternDestroy(pat);
        }
    }

private:
    UniqueFcPattern fTemplate;
    Scenario fScenario;
    SkString fName;
};

// Benchmark measuring pattern duplication/destruction overhead as a baseline control
class FontConfigPatternDupBench : public Benchmark {
public:
    FontConfigPatternDupBench() : fName("fontconfig_pattern_dup_destroy_baseline") {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        fTemplate.reset(FcPatternCreate());
        for (int i = 0; i < 15; ++i) {
            add_pattern_string(fTemplate.get(), FC_FAMILY, "FamilyName", i == 5);
        }
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; ++i) {
            FcPattern* pat = FcPatternDuplicate(fTemplate.get());
            FcPatternDestroy(pat);
        }
    }

private:
    UniqueFcPattern fTemplate;
    SkString fName;
};

// End-to-end benchmark for SkFontMgr::matchFamilyStyle
class FontConfigMatchFamilyStyleBench : public Benchmark {
public:
    FontConfigMatchFamilyStyleBench(const char* familyName)
            : fFamilyName(familyName)
            , fName(SkStringPrintf("fontconfig_matchFamilyStyle_%s", familyName)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override { fFontMgr = ToolUtils::TestFontMgr(); }

    void onDraw(int loops, SkCanvas*) override {
        SkFontStyle style;
        const char* family = fFamilyName.c_str();
        for (int i = 0; i < loops; ++i) {
            sk_sp<SkTypeface> tf = fFontMgr->matchFamilyStyle(family, style);
        }
    }

private:
    SkString fFamilyName;
    SkString fName;
    sk_sp<SkFontMgr> fFontMgr;
};

// End-to-end benchmark for SkFontMgr::matchFamily
class FontConfigMatchFamilyBench : public Benchmark {
public:
    FontConfigMatchFamilyBench(const char* familyName)
            : fFamilyName(familyName)
            , fName(SkStringPrintf("fontconfig_matchFamily_%s", familyName)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override { fFontMgr = ToolUtils::TestFontMgr(); }

    void onDraw(int loops, SkCanvas*) override {
        const char* family = fFamilyName.c_str();
        for (int i = 0; i < loops; ++i) {
            sk_sp<SkFontStyleSet> fss = fFontMgr->matchFamily(family);
        }
    }

private:
    SkString fFamilyName;
    SkString fName;
    sk_sp<SkFontMgr> fFontMgr;
};

DEF_BENCH(return new FontConfigPatternDupBench();)

DEF_BENCH(return new FontConfigRemoveWeakBench(FontConfigRemoveWeakBench::Scenario::kSansSubstitute,
                                               "sans_substitute");)
DEF_BENCH(return new FontConfigRemoveWeakBench(FontConfigRemoveWeakBench::Scenario::kManyElements,
                                               "25_elements");)
DEF_BENCH(return new FontConfigRemoveWeakBench(FontConfigRemoveWeakBench::Scenario::kFewElements,
                                               "2_elements");)
DEF_BENCH(return new FontConfigRemoveWeakBench(FontConfigRemoveWeakBench::Scenario::kAllStrong,
                                               "all_strong");)
DEF_BENCH(return new FontConfigRemoveWeakBench(FontConfigRemoveWeakBench::Scenario::kAllWeak,
                                               "all_weak");)

DEF_BENCH(return new FontConfigMatchFamilyStyleBench("sans-serif");)
DEF_BENCH(return new FontConfigMatchFamilyStyleBench("monospace");)
DEF_BENCH(return new FontConfigMatchFamilyStyleBench("serif");)
DEF_BENCH(return new FontConfigMatchFamilyStyleBench("Roboto");)

DEF_BENCH(return new FontConfigMatchFamilyBench("sans-serif");)
DEF_BENCH(return new FontConfigMatchFamilyBench("monospace");)

#endif  // defined(SK_BUILD_FOR_UNIX) && defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
