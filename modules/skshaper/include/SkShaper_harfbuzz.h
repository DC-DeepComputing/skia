/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_coretext_DEFINED
#define SkShaper_coretext_DEFINED

#include "modules/skshaper/include/SkShaper.h"
#include "include/core/SkRefCnt.h"

#include <memory>

namespace SkShapers {
static std::unique_ptr<SkShaper> MakeShaperDrivenWrapper(sk_sp<SkFontMgr> fallback);
static std::unique_ptr<SkShaper> MakeShapeThenWrap(sk_sp<SkFontMgr> fallback);
static std::unique_ptr<SkShaper> MakeShapeDontWrapOrReorder(std::unique_ptr<SkUnicode> unicode,
                                                            sk_sp<SkFontMgr> fallback);
}

#endif