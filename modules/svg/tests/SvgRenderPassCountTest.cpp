/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <string>
#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/ContextType.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/ganesh/GrContextFactory.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "tools/graphite/ContextFactory.h"
#endif

namespace {

static constexpr const char* kTestSvg = "images/crbug480907585.svg";

static sk_sp<SkSVGDOM> load_test_svg(skiatest::Reporter* reporter,
                                     const char* resource) {
    auto stream = GetResourceAsStream(resource);
    if (!stream) {
        ERRORF(reporter,
               "[SvgRenderPassCountTest] Test SVG '%s' was not found in resources.",
               resource);
        REPORTER_ASSERT(reporter, false, "Test SVG '%s' not found in resources", resource);
        return nullptr;
    }

    auto fontMgr = ToolUtils::TestFontMgr();
    auto dom = SkSVGDOM::Builder().setFontManager(fontMgr).make(*stream);
    if (!dom) {
        ERRORF(reporter, "[SvgRenderPassCountTest] Failed to parse SVG resource '%s'", resource);
        REPORTER_ASSERT(reporter, false, "Failed to parse SVG '%s'", resource);
        return nullptr;
    }

    return dom;
}

#if defined(SK_GANESH)
static void test_svg_render_pass_count_ganesh(skiatest::Reporter* reporter,
                                              const sk_gpu_test::ContextInfo& ctxInfo,
                                              const char* svgPath,
                                              bool layerAtlas,
                                              std::pair<int, int> renderPassRange) {
    auto directContext = ctxInfo.directContext();
    if (!directContext) {
        return;
    }

    auto svgDom = load_test_svg(reporter, svgPath);
    if (!svgDom) {
        return;
    }
    svgDom->setContainerSize(SkSize::Make(1000, 500));

    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(1000, 500);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(directContext, skgpu::Budgeted::kYes, imageInfo);
    REPORTER_ASSERT(reporter, surface != nullptr);
    if (!surface) {
        return;
    }
    surface->getCanvas()->setLayerAtlasEnabled(layerAtlas);

    GrGpu* gpu = directContext->priv().getGpu();
    REPORTER_ASSERT(reporter, gpu != nullptr);
    if (!gpu) {
        return;
    }

    int initialRenderPasses = gpu->stats()->renderPasses();

    svgDom->render(surface->getCanvas());
    directContext->flushAndSubmit();

    int numRenderPasses = gpu->stats()->renderPasses() - initialRenderPasses;

    int minPasses = renderPassRange.first;
    int maxPasses = renderPassRange.second;
    if (directContext->backend() == GrBackendApi::kOpenGL) {
        if (minPasses != -1) {
            minPasses += 1;
        }
        if (maxPasses != -1) {
            maxPasses += 1;
        }
    }

    SkDebugf("[SvgRenderPassCount_Ganesh] SVG: %s, (%s) -> Total renderpasses: %d (expected range: [%d, %d])\n",
             svgPath,
             directContext->backend() == GrBackendApi::kOpenGL ? "OpenGLES" : "Vulkan",
             numRenderPasses, minPasses, maxPasses);
    INFOF(reporter, "[SvgRenderPassCount_Ganesh] SVG: %s, (%s) -> Total renderpasses: %d (expected range: [%d, %d])",
          svgPath,
          directContext->backend() == GrBackendApi::kOpenGL ? "OpenGLES" : "Vulkan",
          numRenderPasses, minPasses, maxPasses);

    if (minPasses != -1) {
        REPORTER_ASSERT(reporter, numRenderPasses >= minPasses,
                        "Expected at least %d render passes for %s in Ganesh %s, got %d",
                        minPasses, svgPath,
                        directContext->backend() == GrBackendApi::kOpenGL ? "OpenGLES" : "Vulkan",
                        numRenderPasses);
    }
    if (maxPasses != -1) {
        REPORTER_ASSERT(reporter, numRenderPasses <= maxPasses,
                        "Expected at most %d render passes for %s in Ganesh %s, got %d",
                        maxPasses, svgPath,
                        directContext->backend() == GrBackendApi::kOpenGL ? "OpenGLES" : "Vulkan",
                        numRenderPasses);
    }
}
#endif

#if defined(SK_GRAPHITE)
static void test_svg_render_pass_count_graphite(skiatest::Reporter* reporter,
                                                skgpu::graphite::Context* context,
                                                const char* svgPath,
                                                bool layerAtlas,
                                                std::pair<int, int> renderPassRange) {
    auto svgDom = load_test_svg(reporter, svgPath);
    if (!svgDom) {
        return;
    }
    svgDom->setContainerSize(SkSize::Make(1000, 500));

    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
    REPORTER_ASSERT(reporter, recorder != nullptr);
    if (!recorder) {
        return;
    }

    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(1000, 500);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    REPORTER_ASSERT(reporter, surface != nullptr);
    if (!surface) {
        return;
    }
    surface->getCanvas()->setLayerAtlasEnabled(layerAtlas);

    svgDom->render(surface->getCanvas());

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    if (!recording) {
        return;
    }

    int numRenderPasses = recording->priv().numRenderPasses();

    int minPasses = renderPassRange.first;
    int maxPasses = renderPassRange.second;

    SkDebugf("[SvgRenderPassCount_Graphite] SVG: %s -> Total renderpasses recorded: %d (expected range: [%d, %d])\n",
             svgPath, numRenderPasses, minPasses, maxPasses);
    INFOF(reporter, "[SvgRenderPassCount_Graphite] SVG: %s -> Total renderpasses recorded: %d (expected range: [%d, %d])",
          svgPath, numRenderPasses, minPasses, maxPasses);

    if (minPasses != -1) {
        REPORTER_ASSERT(reporter, numRenderPasses >= minPasses,
                        "Expected at least %d render passes for %s in Graphite, got %d",
                        minPasses, svgPath, numRenderPasses);
    }
    if (maxPasses != -1) {
        REPORTER_ASSERT(reporter, numRenderPasses <= maxPasses,
                        "Expected at most %d render passes for %s in Graphite, got %d",
                        maxPasses, svgPath, numRenderPasses);
    }

    context->insertRecording({recording.get(), surface.get()});
    context->submit(skgpu::graphite::SyncToCpu::kYes);
}
#endif

static void run_svg_render_pass_count_test(skiatest::Reporter* reporter,
                                           bool layerAtlas,
                                           std::pair<int, int> renderPassRange) {
#if defined(SK_GANESH)
    sk_gpu_test::GrContextFactory ganeshFactory;
    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }
        sk_gpu_test::ContextInfo ctxInfo = ganeshFactory.getContextInfo(contextType);
        if (ctxInfo.directContext()) {
            test_svg_render_pass_count_ganesh(reporter, ctxInfo, kTestSvg, layerAtlas, renderPassRange);
        }
    }
#endif

#if defined(SK_GRAPHITE)
    skiatest::graphite::ContextFactory graphiteFactory;
    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }
        skiatest::graphite::ContextInfo ctxInfo = graphiteFactory.getContextInfo(contextType);
        if (ctxInfo.fContext) {
            test_svg_render_pass_count_graphite(reporter, ctxInfo.fContext, kTestSvg, layerAtlas, renderPassRange);
        }
    }
#endif
}

}  // namespace

DEF_TEST(crbug480907585_renderpass_count_default, reporter) {
    run_svg_render_pass_count_test(reporter, /*layerAtlas=*/false, {430, -1});
}

DEF_TEST(crbug480907585_renderpass_count_filter_atlas, reporter) {
    run_svg_render_pass_count_test(reporter, /*layerAtlas=*/true, {-1, 10});
}
