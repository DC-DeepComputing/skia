/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

// TODO (NEP): Confirm all needed / used.
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

#include <optional>

namespace skgpu::graphite {

/**
 * Implementation of SkTraceMemoryDump which tracks the number of resources whose labels contain
 * a provided string.
 */
class LabelMemoryDump : public SkTraceMemoryDump {
public:
    LabelMemoryDump() = delete;
    ~LabelMemoryDump() override {}
    LabelMemoryDump(SkString labelSubstr) : fLabelStr(labelSubstr) {}

    // Override relevant methods
    void dumpStringValue(const char* dumpName, const char* valueName, const char* value) override {
        if (SkString("type").equals(valueName)) {
            if (SkString(value).contains(fLabelStr.c_str())) {
                fNumResourcesContainingLabel++;
            }
        }
    }

    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }

    bool shouldDump0SizeObjects() const override { return true; }

    // Override other methods to do nothing.
    void setMemoryBacking(const char* dumpName, const char* backingType,
                          const char* backingObjectId) override {}
    void setDiscardableMemoryBacking(
            const char* dumpName, const SkDiscardableMemory& discardableMemoryObject) override {}
    void dumpNumericValue(const char* dumpName, const char* valueName, const char* units,
                          uint64_t value) override {}

    // Define methods useful for this implementation
    uint8_t numResourcesContainingLabel() const { return fNumResourcesContainingLabel; }

private:
    SkString fLabelStr;
    uint8_t fNumResourcesContainingLabel = 0;
};

// Test ABC
// TODO Add support for other backends
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(SampleDstAsInput, reporter, context,
                                     CtsEnforcement::kApiLevel_202504) {
    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(10, 10),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    // ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    // ResourceCache* resourceCache = resourceProvider->resourceCache();
    // Assert the resource cache is empty.
    // REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // Use a blend mode which does not require a dst read.
    static const SkBlendMode simpleBlendMode = SkBlendMode::kSrcATop;
    // Assert the chosen blend mode does not require a dst read. For the sake of this test, assume
    // Coverage::kNone.
    static const Coverage coverage = Coverage::kNone;
    REPORTER_ASSERT(reporter, !IsDstReadRequired(caps, simpleBlendMode, coverage));

    canvas->drawColor(SkColors::kLtGray);//SK_ColorWHITE);
    std::unique_ptr<Recording> clearRecording = recorder->snap();

    canvas->drawColor(SkColors::kDkGray, simpleBlendMode);
    std::unique_ptr<Recording> simpleBlendRecording = recorder->snap();

    LabelMemoryDump rpMemoryDumpNoDstRead { SkString("RenderPass") };
    recorder->dumpMemoryStatistics(&rpMemoryDumpNoDstRead);
    const uint8_t numRenderPassesNoDstRead = rpMemoryDumpNoDstRead.numResourcesContainingLabel();
    REPORTER_ASSERT(reporter, numRenderPassesNoDstRead > 0);

    // Use a blend mode which should expect a dst read.
    static const SkBlendMode dstReadBlendMode = SkBlendMode::kLighten;
    REPORTER_ASSERT(reporter, IsDstReadRequired(caps, dstReadBlendMode, coverage));

    canvas->drawColor(SK_ColorYELLOW, dstReadBlendMode);
    std::unique_ptr<Recording> dstReadRecording = recorder->snap();

    LabelMemoryDump rpMemoryDumpWithDstRead { SkString("RenderPass") };
    recorder->dumpMemoryStatistics(&rpMemoryDumpWithDstRead);

    uint8_t numRenderPassesAfterDstRead = rpMemoryDumpWithDstRead.numResourcesContainingLabel();
    REPORTER_ASSERT(reporter,
                    numRenderPassesNoDstRead == numRenderPassesAfterDstRead,
                    "# renderpasses with no dst read: %u ; after dst read: %u",
                    (unsigned)numRenderPassesNoDstRead,
                    (unsigned)numRenderPassesAfterDstRead);
}

}  // namespace skgpu::graphite
