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
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
            const char* utf8, size_t utf8Bytes, SkFourByteTag script) const = 0;
    virtual SkUnicode* getUnicode() const = 0;
};

namespace Primitive {
SK_API sk_sp<Factory> Factory();
}

SK_API const sk_sp<Factory>& GetFactory();
SK_API void SetFactory(sk_sp<Factory>);

}  // namespace SkShapers

#endif  // SkShaperFactory_DEFINED
