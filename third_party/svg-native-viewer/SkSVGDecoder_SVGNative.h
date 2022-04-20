/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDecoder_SVGNative_DEFINED
#define SkSVGDecoder_SVGNative_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSVGDecoder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

class SkCanvas;

namespace SVGNative {
    class SVGDocument;
    class SkiaSVGRenderer;
}

class SkSVGDecoder_SVGNative : public SkSVGDecoder {
public:
    static std::unique_ptr<SkSVGDecoder> Make(const uint8_t* svg, size_t svgLength);
    size_t approximateSize() override;
    bool render(SkCanvas&, int upem, SkGlyphID glyphId,
                SkColor foregroundColor, SkSpan<SkColor> palette) override;
    ~SkSVGDecoder_SVGNative() override;
private:
    SkSVGDecoder_SVGNative(std::unique_ptr<SVGNative::SVGDocument>,
                           std::shared_ptr<SVGNative::SkiaSVGRenderer>,
                           size_t approximateSize);
    std::unique_ptr<SVGNative::SVGDocument> fDocument;
    std::shared_ptr<SVGNative::SkiaSVGRenderer> fRenderer;
    size_t fApproximateSize;
};

#endif  // SkSVGDecoder_SVGNative
