/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDecoder_DEFINED
#define SkSVGDecoder_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

class SkCanvas;

class SkSVGDecoder {
public:
    virtual bool render(SkCanvas&,
                        const uint8_t* svg, size_t svgLength, int upem, SkGlyphID glyphId,
                        SkColor foregroundColor, SkSpan<SkColor> palette) = 0;
    virtual ~SkSVGDecoder() = default;
};

#endif  // SkSVGDecoder_DEFINED
