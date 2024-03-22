/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShaperFactory_DEFINED
#define SkShaperFactory_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkFontMgr;
class SkUnicode;

namespace SkShapers {

class SK_API Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) const = 0;
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(
            const char* utf8, size_t utf8Bytes) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(
            const char* utf8, size_t utf8Bytes, SkFourByteTag script) const = 0;
    virtual SkUnicode* getUnicode() const = 0;
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
    std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(const char*,
                                                                 size_t,
                                                                 SkFourByteTag) const override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    }
    SkUnicode* getUnicode() const override { return nullptr; }
};
}  // namespace priv

namespace Primitive {
inline sk_sp<Factory> Factory() { return sk_make_sp<SkShapers::priv::PrimitiveFactory>(); }
}  // namespace Primitive

SK_API sk_sp<Factory> GetFactory();
SK_API void RegisterFactory(sk_sp<Factory>);

}  // namespace SkShapers

#endif  // SkShaperFactory_DEFINED
