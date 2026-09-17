/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
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

#if defined(SK_GRAPHITE)
#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/TestOptions.h"
#endif

#include <cstring>
#include <memory>
#include <string>
#include <utility>

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

#if defined(SK_GRAPHITE)
void enable_scratch_device_atlas(skgpu::graphite::ContextOptions* options) {
    options->fUseScratchDeviceAtlas = true;
}

class GpuMemoryTracker : public SkTraceMemoryDump {
public:
    GpuMemoryTracker() = default;
    ~GpuMemoryTracker() override = default;

    void dumpNumericValue(const char* dumpName,
                          const char* valueName,
                          const char* units,
                          uint64_t value) override {
        if (SkString("size").equals(valueName) && SkString("bytes").equals(units)) {
            fTotalBytes += value;
            fLastSizeDumpName = dumpName;
            fLastSizeValue = value;
        }
    }

    void dumpStringValue(const char* dumpName,
                         const char* valueName,
                         const char* value) override {
        if (SkString("type").equals(valueName) && SkString(value).contains("Texture")) {
            fTextureCount++;
            if (fLastSizeDumpName.equals(dumpName)) {
                fTextureBytes += fLastSizeValue;
            }
        }
    }

    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }

    bool shouldDumpSizelessObjects() const override { return false; }

    void setMemoryBacking(const char*, const char*, const char*) override {}
    void setDiscardableMemoryBacking(const char*, const SkDiscardableMemory&) override {}

    uint64_t totalBytes() const { return fTotalBytes; }
    uint64_t textureBytes() const { return fTextureBytes; }
    int textureCount() const { return fTextureCount; }

private:
    uint64_t fTotalBytes = 0;
    uint64_t fTextureBytes = 0;
    int fTextureCount = 0;
    SkString fLastSizeDumpName;
    uint64_t fLastSizeValue = 0;
};
#endif

}  // namespace

#if defined(SK_GRAPHITE)
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(SvgRenderPassCount_Graphite,
                                           skgpu::IsRenderingContext,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_scratch_device_atlas,
                                           /* condition= */ true,
                                           CtsEnforcement::kNever) {
    auto svgDom = load_test_svg(reporter, kTestSvg);
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

    svgDom->render(surface->getCanvas());

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, recording != nullptr);
    if (!recording) {
        return;
    }

    int numRenderPasses = recording->priv().numRenderPasses();
    INFOF(reporter, "[SvgRenderPassCount_Graphite] SVG: %s -> Total renderpasses recorded: %d",
          kTestSvg, numRenderPasses);

    // Without atlasing, 438+ passes. With ScratchDeviceAtlas, <= 8 passes.
    REPORTER_ASSERT(reporter, numRenderPasses <= 8,
                    "Expected <= 8 render passes for %s in Graphite, got %d",
                    kTestSvg, numRenderPasses);

    context->insertRecording({recording.get(), surface.get()});
    context->submit(skgpu::graphite::SyncToCpu::kYes);
}

DEF_GRAPHITE_TEST(SvgMemoryUsage_Graphite, reporter, CtsEnforcement::kNever) {
    auto svgDom = load_test_svg(reporter, kTestSvg);
    if (!svgDom) {
        return;
    }
    svgDom->setContainerSize(SkSize::Make(1000, 500));

    struct RunStats {
        int fRenderPasses = 0;
        size_t fRecorderBudgetedBytesAfterSnap = 0;
        size_t fContextBudgetedBytesAfterSubmit = 0;
        uint64_t fDumpTotalBytes = 0;
        uint64_t fDumpTextureBytes = 0;
        int fDumpTextureCount = 0;
    };

    auto measureSvgRender = [&](skgpu::ContextType contextType,
                                bool useScratchAtlas,
                                RunStats* outStats) -> bool {
        skiatest::graphite::TestOptions testOptions;
        testOptions.fContextOptions.fUseScratchDeviceAtlas = useScratchAtlas;
        skiatest::graphite::ContextFactory factory(testOptions);
        skiatest::graphite::ContextInfo ctxInfo = factory.getContextInfo(contextType);
        if (!ctxInfo.fContext) {
            return false;
        }
        skgpu::graphite::Context* context = ctxInfo.fContext;

        std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();
        REPORTER_ASSERT(reporter, recorder != nullptr);
        if (!recorder) {
            return false;
        }

        SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(1000, 500);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), imageInfo);
        REPORTER_ASSERT(reporter, surface != nullptr);
        if (!surface) {
            return false;
        }

        svgDom->render(surface->getCanvas());

        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
        REPORTER_ASSERT(reporter, recording != nullptr);
        if (!recording) {
            return false;
        }

        outStats->fRenderPasses = recording->priv().numRenderPasses();
        outStats->fRecorderBudgetedBytesAfterSnap = recorder->currentBudgetedBytes();

        context->insertRecording({recording.get(), surface.get()});
        context->submit(skgpu::graphite::SyncToCpu::kYes);

        outStats->fContextBudgetedBytesAfterSubmit =
                context->currentBudgetedBytes() + recorder->currentBudgetedBytes();

        GpuMemoryTracker tracker;
        context->dumpMemoryStatistics(&tracker);
        recorder->dumpMemoryStatistics(&tracker);
        outStats->fDumpTotalBytes = tracker.totalBytes();
        outStats->fDumpTextureBytes = tracker.textureBytes();
        outStats->fDumpTextureCount = tracker.textureCount();
        return true;
    };

    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (!skgpu::IsRenderingContext(contextType)) {
            continue;
        }

        RunStats withoutAtlas;
        RunStats withAtlas;
        if (!measureSvgRender(contextType, /*useScratchAtlas=*/false, &withoutAtlas) ||
            !measureSvgRender(contextType, /*useScratchAtlas=*/true, &withAtlas)) {
            continue;
        }

        INFOF(reporter,
              "[SvgMemoryUsage_Graphite] Without ScratchDeviceAtlas: "
              "renderPasses=%d, budgetedAfterSnap=%zu bytes (%.2f MB), "
              "budgetedAfterSubmit=%zu bytes (%.2f MB), "
              "dumpTotal=%llu bytes (%.2f MB), dumpTextures=%d (%llu bytes / %.2f MB)\n",
              withoutAtlas.fRenderPasses,
              withoutAtlas.fRecorderBudgetedBytesAfterSnap,
              withoutAtlas.fRecorderBudgetedBytesAfterSnap / (1024.0 * 1024.0),
              withoutAtlas.fContextBudgetedBytesAfterSubmit,
              withoutAtlas.fContextBudgetedBytesAfterSubmit / (1024.0 * 1024.0),
              static_cast<unsigned long long>(withoutAtlas.fDumpTotalBytes),
              withoutAtlas.fDumpTotalBytes / (1024.0 * 1024.0),
              withoutAtlas.fDumpTextureCount,
              static_cast<unsigned long long>(withoutAtlas.fDumpTextureBytes),
              withoutAtlas.fDumpTextureBytes / (1024.0 * 1024.0));

        INFOF(reporter,
              "[SvgMemoryUsage_Graphite] With ScratchDeviceAtlas:    "
              "renderPasses=%d, budgetedAfterSnap=%zu bytes (%.2f MB), "
              "budgetedAfterSubmit=%zu bytes (%.2f MB), "
              "dumpTotal=%llu bytes (%.2f MB), dumpTextures=%d (%llu bytes / %.2f MB)\n",
              withAtlas.fRenderPasses,
              withAtlas.fRecorderBudgetedBytesAfterSnap,
              withAtlas.fRecorderBudgetedBytesAfterSnap / (1024.0 * 1024.0),
              withAtlas.fContextBudgetedBytesAfterSubmit,
              withAtlas.fContextBudgetedBytesAfterSubmit / (1024.0 * 1024.0),
              static_cast<unsigned long long>(withAtlas.fDumpTotalBytes),
              withAtlas.fDumpTotalBytes / (1024.0 * 1024.0),
              withAtlas.fDumpTextureCount,
              static_cast<unsigned long long>(withAtlas.fDumpTextureBytes),
              withAtlas.fDumpTextureBytes / (1024.0 * 1024.0));

        REPORTER_ASSERT(reporter, withoutAtlas.fRenderPasses > 100,
                        "Expected > 100 render passes without atlas, got %d",
                        withoutAtlas.fRenderPasses);
        REPORTER_ASSERT(reporter, withAtlas.fRenderPasses <= 8,
                        "Expected <= 8 render passes with atlas, got %d",
                        withAtlas.fRenderPasses);
        REPORTER_ASSERT(reporter, withoutAtlas.fRecorderBudgetedBytesAfterSnap > 0);
        REPORTER_ASSERT(reporter, withAtlas.fRecorderBudgetedBytesAfterSnap > 0);
        REPORTER_ASSERT(reporter, withAtlas.fDumpTextureCount < withoutAtlas.fDumpTextureCount,
                        "Expected fewer textures allocated with atlas (%d) than without (%d)",
                        withAtlas.fDumpTextureCount, withoutAtlas.fDumpTextureCount);
    }
}
#endif

DEF_TEST(SvgRenderContext_StrokeOBBAndFilterNoneOpacity, reporter) {
    // 1. Verify that a node with both fill and a wide stroke (stroke-width=20) plus opacity="0.5"
    // or filter="url(#f)" outsets its saveLayer bounds by the stroke inflation radius so the
    // stroke outside the geometry OBB [20, 20, 80, 80] is not clipped.
    // 2. Verify that a node with filter="none" and opacity="0.5" still applies opacity.
    static constexpr const char* kSvgSrc = R"svg(
        <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
            <defs>
                <filter id="ident" x="-50%" y="-50%" width="200%" height="200%">
                    <feOffset dx="0" dy="0"/>
                </filter>
            </defs>
            <rect x="20" y="20" width="40" height="40"
                  fill="#00ff00" stroke="#ff0000" stroke-width="20" opacity="0.5"/>
            <rect x="90" y="20" width="40" height="40"
                  fill="#00ff00" stroke="#0000ff" stroke-width="20" filter="url(#ident)"/>
            <rect x="150" y="20" width="40" height="40"
                  fill="#ff0000" filter="none" opacity="0.5"/>
        </svg>
    )svg";

    SkMemoryStream stream(kSvgSrc, std::strlen(kSvgSrc));
    auto dom = SkSVGDOM::Builder().make(stream);
    REPORTER_ASSERT(reporter, dom != nullptr);
    if (!dom) {
        return;
    }

    SkBitmap bm;
    bm.allocN32Pixels(200, 100);
    bm.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(bm);
    dom->render(&canvas);

    // Rect 1 has geometry OBB [20, 20, 60, 60] and stroke-width 20, so stroke extends to
    // [10, 10, 70, 70]. Sample at (15, 40) which is outside the raw OBB but inside the stroke.
    SkColor strokeOpacityColor = bm.getColor(15, 40);
    REPORTER_ASSERT(reporter, SkColorGetA(strokeOpacityColor) > 100 &&
                              SkColorGetR(strokeOpacityColor) > 200,
                    "Expected visible red stroke outside raw OBB for opacity layer, got 0x%08x",
                    strokeOpacityColor);

    // Rect 2 has geometry OBB [90, 20, 130, 60] and stroke-width 20, so stroke extends to
    // [80, 10, 140, 70]. Sample at (85, 40) which is outside the raw OBB but inside the stroke.
    SkColor strokeFilterColor = bm.getColor(85, 40);
    REPORTER_ASSERT(reporter, SkColorGetA(strokeFilterColor) > 200 &&
                              SkColorGetB(strokeFilterColor) > 200,
                    "Expected visible blue stroke outside raw OBB for filter layer, got 0x%08x",
                    strokeFilterColor);

    // Rect 3 at [150, 20, 190, 60] has filter="none" and opacity="0.5".
    // Sample at (170, 40) and verify alpha is ~128 (not 255).
    SkColor filterNoneOpacityColor = bm.getColor(170, 40);
    uint32_t alpha = SkColorGetA(filterNoneOpacityColor);
    REPORTER_ASSERT(reporter, alpha >= 110 && alpha <= 145,
                    "Expected ~50%% alpha for filter=\"none\" opacity=\"0.5\", got alpha=%u",
                    alpha);
}
