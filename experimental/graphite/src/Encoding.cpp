/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Encoding.h"

#include "include/private/SkCode.h"

class SkMagicCodeDictionary;

int BlendMode::Encode1(SkMagicCodeDictionary*, Mapping mapping, SkCode* code, SkBlendMode bm) {
    return 0;
}

void BlendMode::Dump(const uint32_t* data, int maxIndex) {
    SkASSERT(maxIndex == 2);
    SkASSERT(Foo::kBlendMode == static_cast<Foo>(data[0] & kFooMask));

    SkDebugf("%s ", BlendModeToStr(static_cast<SkBlendMode>(data[1])));
}
