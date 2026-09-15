/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_coretext_DEFINED
#define SkShaper_coretext_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_factory.h"

#include <memory>

class SkUnicode;

namespace SkShapers::CT {

enum class LineBreakMode {
    kDefault, // Default CT line breaking, may split long words.
    kStrict,  // Never splits words across multiple lines, but can overflow the paragraph width.
};

SKSHAPER_API std::unique_ptr<SkShaper> CoreText(LineBreakMode = LineBreakMode::kDefault);
SKSHAPER_API sk_sp<SkShapers::Factory> Factory(sk_sp<SkUnicode> unicode,
                                               LineBreakMode = LineBreakMode::kDefault);

}  // namespace SkShapers::CT

#endif  // SkShaper_coretext_DEFINED
