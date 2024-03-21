/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGShapingUtils_DEFINED
#define SkSVGShapingUtils_DEFINED

#include "modules/skshaper/include/SkShaper.h"
#include "modules/svg/include/SkSVGShaping.h"

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#endif

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
#include "modules/skshaper/include/SkShaper_coretext.h"
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

namespace SkSVGShaping {
// This convenience function will return a set of callbacks that has the "best" text shaping
// depending on what parts of Skia the client has compiled in. For example, if the clients
// have compiled in SkShaper and a version of SkUnicode, callbacks which produce the
// appropriate types will be returned.
//
// This must be inline because the client has to compile this code with all defines
// set by *their* dependencies (which may include defines from SkShaper modules).
inline Callbacks BestAvailable() {
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
    // This is defined as a lambda because we don't want it exposed as part of the API.
    auto getUnicode = []() -> sk_sp<SkUnicode> {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
        if (auto unicode = SkUnicodes::ICU::Make()) {
            return unicode;
        }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
        if (auto unicode = SkUnicodes::ICU4X::Make()) {
            return unicode;
        }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
        if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
            return unicode;
        }
#endif
        return nullptr;
    };

    auto shaperCallback = [&](sk_sp<SkFontMgr> fallback) -> std::unique_ptr<SkShaper> {
        return SkShapers::HB::ShaperDrivenWrapper(getUnicode(), fallback);
    };
    auto bidiCallback = [&](const char* utf8,
                            size_t utf8Bytes,
                            uint8_t bidiLevel) -> std::unique_ptr<SkShaper::BiDiRunIterator> {
        const SkBidiIterator::Level defaultLevel = SkBidiIterator::kLTR;
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
                SkShapers::unicode::BidiRunIterator(getUnicode(), utf8, utf8Bytes, defaultLevel);
        return bidi;
    };
    auto runCallback = [](const char* utf8,
                          size_t utf8Bytes) -> std::unique_ptr<SkShaper::ScriptRunIterator> {
        return SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes);
    };
    return std::make_tuple(shaperCallback, bidiCallback, runCallback);

#elif defined(SK_SHAPER_CORETEXT_AVAILABLE)
    auto shaperCallback = [](sk_sp<SkFontMgr> fallback) -> std::unique_ptr<SkShaper> {
        return SkShapers::CT::CoreText();
    };
    auto bidiCallback = [](const char*,
                           size_t,
                           uint8_t) -> std::unique_ptr<SkShaper::BiDiRunIterator> {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    };
    auto runCallback = [](const char*,
                          size_t) -> std::unique_ptr<SkShaper::ScriptRunIterator> {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    };
    return std::make_tuple(shaperCallback, bidiCallback, runCallback);

#else
    return Primitive();
#endif
}
};  // namespace SkSVGShaping

#endif  // SkSVGShapingUtils_DEFINED
