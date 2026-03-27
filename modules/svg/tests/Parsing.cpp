/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGRenderContext.h"
#include "tests/Test.h"

DEF_TEST(Svg_Parsing_ParsingRunsWithDifferentLocale, r) {
    auto* original_locale = std::setlocale(LC_NUMERIC, "ru_RU.UTF-8");
    const std::string svgText = R"EOF(
    <svg width="100" height="100" xmlns="http://www.w3.org/2000/svg">
      <path id='pp' d="M10.2 10.6" />
     </svg>
    )EOF";
    auto str = SkMemoryStream::MakeDirect(svgText.c_str(), svgText.size());
    std::setlocale(LC_NUMERIC, original_locale);
    auto svg_dom = SkSVGDOM::MakeFromStream(*str);
    REPORTER_ASSERT(r, svg_dom != nullptr);
    sk_sp<SkSVGNode>* node = svg_dom->findNodeById("pp");
    REPORTER_ASSERT(r, node != nullptr);
    SkSVGLengthContext lctx(svg_dom->containerSize());
    SkSVGPresentationContext pctx;
    SkCanvas fake_canvas;
    SkSVGIDMapper fIDMapper;
    SkPath path = (*node)->asPath(
            SkSVGRenderContext(&fake_canvas, nullptr, nullptr, fIDMapper,
                               lctx, pctx, {nullptr, nullptr}, nullptr));
    REPORTER_ASSERT(r, !path.isEmpty());
}
