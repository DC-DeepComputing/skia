/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextProvider_DEFINED
#define SkTextProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkFontMgr;
namespace SkShapers { class Factory; }

namespace skresources {
class SK_API TextProvider : public SkRefCnt {
public:
    virtual sk_sp<SkFontMgr> fontMgr() const = 0;

    virtual sk_sp<SkShapers::Factory> textShapingFactory() const = 0;
};

}  // namespace skresources

#endif  // SkTextProvider_DEFINED
