/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGShaping_DEFINED
#define SkSVGShaping_DEFINED

#include "include/private/base/SkAPI.h"
#include "modules/skshaper/include/SkShaper.h"

#include <functional>

namespace SkSVGShaping {
// We define these as std::function because it's conveneint to be able to use lambdas with captures for these.
using MakeShaperCallback = std::function<std::unique_ptr<SkShaper>(sk_sp<SkFontMgr> fallback)>;
using MakeBidiIteratorCallback = std::function<std::unique_ptr<SkShaper::BiDiRunIterator>(const char* utf8,
                                                                                size_t utf8Bytes,
                                                                                uint8_t bidiLevel)>;
using MakeRunIteratorCallback = std::function<std::unique_ptr<SkShaper::ScriptRunIterator>(const char* utf8,
                                                                                 size_t utf8Bytes)>;
using Callbacks = std::tuple<MakeShaperCallback, MakeBidiIteratorCallback, MakeRunIteratorCallback>;

inline Callbacks Primitive() {
    auto shaperCallback = [](sk_sp<SkFontMgr>) -> std::unique_ptr<SkShaper> {
        return SkShapers::Primitive::PrimitiveText();
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
}

};  // namespace SkSVGShaping

#endif  // SkSVGShaping_DEFINED
