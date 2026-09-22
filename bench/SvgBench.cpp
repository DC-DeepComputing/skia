/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#if defined(SK_ENABLE_SVG)

#include <cstring>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

namespace {

static constexpr const char* kBenchFile = "images/crbug480907585.svg";

static sk_sp<SkSVGDOM> load_test_svg(const char* resource) {
    auto stream = GetResourceAsStream(resource);
    if (!stream) {
        SK_ABORT("[SvgBench] Benchmark SVG '%s' was not found in resources.", resource);
        return nullptr;
    }

    auto fontMgr = ToolUtils::TestFontMgr();
    auto dom = SkSVGDOM::Builder().setFontManager(fontMgr).make(*stream);
    if (!dom) {
        SK_ABORT("[SvgBench] Failed to parse benchmark SVG '%s'", resource);
        return nullptr;
    }

    return dom;
}

}  // namespace

class SvgBench : public Benchmark {
public:
    SvgBench(bool layerAtlas)
        : flayerAtlas(layerAtlas) {
        fName.printf("crbug480907585_bench_%s", flayerAtlas ? "atlas" : "default");
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkISize onGetSize() override {
        return SkISize::Make(1000, 500);
    }

    void onDelayedSetup() override {
        fDom = load_test_svg(kBenchFile);
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(1000, 500));
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        canvas->setLayerAtlasEnabled(flayerAtlas);
        for (int i = 0; i < loops; ++i) {
            canvas->clear(SkColorSetRGB(0x0b, 0x0e, 0x14));
            if (fDom) {
                fDom->render(canvas);
            }
        }
    }

private:
    SkString fName;
    bool flayerAtlas;
    sk_sp<SkSVGDOM> fDom;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new SvgBench(/*layerAtlas=*/true); )
DEF_BENCH( return new SvgBench(/*layerAtlas=*/false); )

#endif // SK_ENABLE_SVG
