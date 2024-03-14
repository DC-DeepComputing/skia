/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGShape_DEFINED
#define SkSVGShape_DEFINED

#include "include/private/base/SkAPI.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/svg/include/SkSVGRenderContext.h"

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#endif

namespace SkSVGShaping {
using MakeShaperCallback = SkSVGRenderContext::MakeShaperCallback;
using MakeBidiIteratorCallback = SkSVGRenderContext::MakeBidiIteratorCallback;
using MakeRunIteratorCallback = SkSVGRenderContext::MakeRunIteratorCallback;
using ShapingCallbacks = SkSVGRenderContext::ShapingCallbacks;

inline ShapingCallbacks Primitive() {
    auto shaperCallback = [](sk_sp<SkFontMgr>) -> std::unique_ptr<SkShaper> {
        return SkShapers::Primitive();
    };
    auto bidiCallback = [](const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) -> std::unique_ptr<SkShaper::BiDiRunIterator> {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    };
    auto runCallback = [](const char* utf8, size_t utf8Bytes) -> std::unique_ptr<SkShaper::ScriptRunIterator> {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    };
    return std::make_tuple(shaperCallback, bidiCallback, runCallback);
}

// This must be inline because the client has to compile this code with all defines
// set by *their* dependencies (which may include defines from SkShaper modules).
inline ShapingCallbacks HarfbuzzIfAvailable() {
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
    auto shaperCallback = [](sk_sp<SkFontMgr> fallback) -> std::unique_ptr<SkShaper> {
        auto unicode = SkUnicode::Make();
        return SkShapers::HB::ShaperDrivenWrapper(std::move(unicode), fallback);
    };
    auto bidiCallback = [](const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) -> std::unique_ptr<SkShaper::BiDiRunIterator>{
        auto unicode = SkUnicode::Make();
        const SkBidiIterator::Level defaultLevel = SkBidiIterator::kLTR;
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
                SkShapers::unicode::BidiRunIterator(unicode.get(), utf8, utf8Bytes, defaultLevel);
        return bidi;
    };
    auto runCallback = [](const char* utf8, size_t utf8Bytes) -> std::unique_ptr<SkShaper::ScriptRunIterator> {
        return SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes);
    };
    return std::make_tuple(shaperCallback, bidiCallback, runCallback);
#else
    return Primitive();
#endif
}
};

#endif // SkSVGShape_DEFINED
