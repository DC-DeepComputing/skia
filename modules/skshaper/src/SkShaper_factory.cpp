/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper_factory.h"

#include <utility>

namespace {
class PrimitiveFactory : public SkShapers::Factory {
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const override {
        return SkShapers::Primitive::PrimitiveText();
    }

    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(const char*,
                                                                   size_t,
                                                                   uint8_t) const override {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    }

    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
            const char*, size_t, SkFourByteTag) const override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    }

    SkUnicode* getUnicode() const override { return nullptr; }
};
}  // namespace

namespace SkShapers {

namespace Primitive {
sk_sp<SkShapers::Factory> Factory() {
    return sk_make_sp<PrimitiveFactory>();
}
}  // namespace Primitive

// A static variable inside a function avoids a static initializer.
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/static_initializers.md#removing-static-initializers
static sk_sp<Factory>* get_factory() {
    static sk_sp<Factory> factory = SkShapers::Primitive::Factory();
    return &factory;
}

const sk_sp<Factory>& GetFactory() {
    return *get_factory();
}

void SetFactory(sk_sp<Factory> newFactory) {
    *get_factory() = std::move(newFactory);
}

}  // namespace SkShapers
