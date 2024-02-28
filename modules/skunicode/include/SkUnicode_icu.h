/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_icu_DEFINED
#define SkUnicode_icu_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#include <memory>

namespace SkUnicodes::ICU {
SKUNICODE_API std::unique_ptr<SkUnicode> Make();
SKUNICODE_API std::unique_ptr<SkUnicode> MakeFromRuntimeLoad();
}

#endif //SkUnicode_icu_DEFINED
