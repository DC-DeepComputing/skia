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

#include <memory>

class SkCanvas;

class SkSVGDecoder {
public:
    /** An SkSVGDecoder instance probably owns an SVG DOM. The SkSVGDecoder may be cached with the
     *  glyph, and as such needs to account for how much memory it retains.
     */
    virtual size_t approximateSize() = 0;
    virtual bool render(SkCanvas&, int upem, SkGlyphID glyphId,
                        SkColor foregroundColor, SkSpan<SkColor> palette) = 0;
    virtual ~SkSVGDecoder() = default;
};

#endif  // SkSVGDecoder_DEFINED
