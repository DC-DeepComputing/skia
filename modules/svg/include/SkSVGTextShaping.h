/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGTextShaping_DEFINED
#define SkSVGTextShaping_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "modules/skshaper/include/SkShaper.h"

#include <memory>

namespace SkSVGTextShaping {

class SK_API Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) const = 0;
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(
            const char* utf8, size_t utf8Bytes) const = 0;
};

namespace priv {
class PrimitiveFactory : public Factory {
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const override {
        return SkShapers::Primitive::PrimitiveText();
    }
    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiIterator(const char*,
                                                                size_t,
                                                                uint8_t) const override {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    }
    std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(const char*,
                                                                 size_t) const override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    }
};
}  // namespace priv

inline sk_sp<Factory> Primitive() { return sk_make_sp<SkSVGTextShaping::priv::PrimitiveFactory>(); }

}  // namespace SkSVGTextShaping

#endif  // SkSVGTextShaping_DEFINED
