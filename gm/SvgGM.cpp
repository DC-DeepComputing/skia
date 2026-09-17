/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/ContextOptions.h"
#endif

namespace skiagm {

class SvgGM : public GM {
public:
    SvgGM(const char* name, const char* svgPath)
            : fName(name)
            , fSvgPath(svgPath) {
        this->setBGColor(SkColorSetRGB(0x0b, 0x0e, 0x14));
    }

#if defined(SK_GRAPHITE)
    void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions* options) const override {
        options->fUseScratchDeviceAtlas = true;
    }
#endif

protected:
    SkString getName() const override { return fName; }

    SkISize getISize() override { return SkISize::Make(1000, 500); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto stream = GetResourceAsStream(fSvgPath.c_str());
        if (!stream) {
            *errorMsg = SkStringPrintf("SVG resource '%s' not found.", fSvgPath.c_str());
            return DrawResult::kFail;
        }

        auto fontMgr = ToolUtils::TestFontMgr();
        auto dom = SkSVGDOM::Builder()
                           .setFontManager(fontMgr)
                           .make(*stream);
        if (!dom) {
            *errorMsg = SkStringPrintf("Failed to parse SVG: %s", fSvgPath.c_str());
            return DrawResult::kFail;
        }

        dom->setContainerSize(SkSize::Make(1000, 500));

        dom->render(canvas);

        return DrawResult::kOk;
    }

private:
    SkString fName;
    SkString fSvgPath;
};

DEF_GM(return new SvgGM("crbug480907585_render", "images/crbug480907585.svg");)
DEF_GM(return new SvgGM("test_both", "images/test_both.svg");)
DEF_GM(return new SvgGM("test_far", "images/test_far.svg");)
DEF_GM(return new SvgGM("test_frac_far", "images/test_frac_far.svg");)
DEF_GM(return new SvgGM("test_int_overlap", "images/test_int_overlap.svg");)
DEF_GM(return new SvgGM("test_rx_int", "images/test_rx_int.svg");)
DEF_GM(return new SvgGM("test_frac_norx", "images/test_frac_norx.svg");)

}  // namespace skiagm

