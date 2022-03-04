/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkSVGDecoder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkEnumerate.h"
#include "SkSVGDecoder_SVGNative.h"

#include "svgnative/SVGDocument.h"
#include "svgnative/SVGRenderer.h"
#include "svgnative/ports/skia/SkiaSVGRenderer.h"

#include <memory>

bool SkSVGDecoder_SVGNative::render(SkCanvas& canvas,
                                    const uint8_t* svg, size_t svgLength,
                                    int upem, SkGlyphID glyphId,
                                    SkColor foregroundColor, SkSpan<SkColor> palette) {
    // svgnative requires a '\0' teminated UTF-8 string, so a copy must be made to add it.
    SkString svgString((const char*)svg, svgLength);

    auto renderer = std::make_shared<SVGNative::SkiaSVGRenderer>();
    auto snv_document = std::unique_ptr<SVGNative::SVGDocument>(
        SVGNative::SVGDocument::CreateSVGDocument(svgString.c_str(), renderer));
    if (!snv_document) {
        return false;
    }

    constexpr const size_t glyphStringLen = sizeof("glyph") - 1;
    char glyphIdString[glyphStringLen + kSkStrAppendU32_MaxSize + 1] = "glyph";
    *SkStrAppendU32(glyphIdString + glyphStringLen, glyphId) = 0;

    renderer->SetSkCanvas(&canvas);
    SkColor4f cc = SkColor4f::FromColor(foregroundColor);
    SVGNative::ColorMap colorMap = {
        {"currentColor", std::array<float, 4>{{cc.fR,  cc.fG, cc.fB, cc.fA}}},
    };
    for (auto&& [i, color] : SkMakeEnumerate(palette)) {
        SkColor4f pc = SkColor4f::FromColor(color);

        constexpr const size_t colorStringLen = sizeof("color") - 1;
        char colorIdString[colorStringLen + kSkStrAppendU32_MaxSize + 1] = "color";
        *SkStrAppendU32(colorIdString + colorStringLen, i) = 0;

        colorMap.emplace(colorIdString, std::array<float, 4>{{pc.fR,  pc.fG, pc.fB, pc.fA}});
    }
    snv_document->Render(glyphIdString, colorMap, upem, upem);

    return true;
}
