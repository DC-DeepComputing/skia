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
        if (SkString("label").equals(valueName) && SkString(value).contains(fLabelStr.c_str())) {
            fNumResourcesContainingLabel++;
        }
    }

    LevelOfDetail getRequestedDetails() const override {
        return SkTraceMemoryDump::kObjectsBreakdowns_LevelOfDetail;
    }
    bool shouldDumpWrappedObjects() const override { return true; }

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
    uint8_t fNumResourcesContainingLabel;
};

// Test ABC
// TODO Add support for other backends
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(SampleDstAsInput, reporter, context,
                                     CtsEnforcement::kApiLevel_202504) {
    const Caps* caps = context->priv().caps();

    // Renderpasses are not graphite-level objects at this point in time

    // resource -> getResourceType
    // recorder->dumpMemoryStatistics



    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    SkImageInfo ii = SkImageInfo::Make(SkISize::Make(10, 10),
                                       SkColorType::kRGBA_8888_SkColorType,
                                       SkAlphaType::kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    // Assert the resource cache is empty.
    REPORTER_ASSERT(reporter, resourceCache->getResourceCount() == 0);

    // Use a blend mode which does not require a dst read.
    static const SkBlendMode simpleBlendMode = SkBlendMode::kSrc;
    // Assert the chosen blend mode does not require a dst read. For the sake of this test, assume
    // Coverage::kNone.
    static const Coverage coverage = Coverage::kNone;
    REPORTER_ASSERT(reporter, !IsDstReadRequired(caps, simpleBlendMode, coverage));

    // simplePaint.setBlendMode(simpleBlendMode);
    // simplePaint.setColor(SkColor::SK_ColorMAGENTA);
    canvas->drawColor(SK_ColorMAGENTA, simpleBlendMode);

    LabelMemoryDump rpMemoryDumpNoDstRead { SkString("RenderPass") };
    recorder->dumpMemoryStatistics(&rpMemoryDumpNoDstRead);

    // Use a blend mode which should expect a dst read.
    static const SkBlendMode dstReadBlendMode = SkBlendMode::kLighten;
    REPORTER_ASSERT(reporter, IsDstReadRequired(caps, dstReadBlendMode, coverage));

    // TextureInfo texInfo;
    // texInfo.
    // REPORTER_ASSERT(caps->getDstReadStrategy() == DstReadStrategy::kReadFromInput);
    canvas->drawColor(SK_ColorYELLOW, dstReadBlendMode);


    LabelMemoryDump rpMemoryDumpWithDstRead { SkString("RenderPass") };
    recorder->dumpMemoryStatistics(&rpMemoryDumpWithDstRead);

    REPORTER_ASSERT(reporter, rpMemoryDumpWithDstRead.numResourcesContainingLabel() ==
                              rpMemoryDumpNoDstRead.numResourcesContainingLabel());


    // Create a target surface that should support reading it as an input attachment in Vulkan.
    // const SkImageInfo surfaceImageInfo = SkImageInfo::Make(
    //         16, 16, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
    // TextureInfo dstTextureInfo =
    //         caps->getDefaultSampledTextureInfo(SkColorType::kRGBA_8888_SkColorType,
    //                                            Mipmapped::kNo,
    //                                            Protected::kNo,
    //                                            Renderable::kYes);
    // SkImageInfo surfaceInfo = SkImageInfo::Make({16, 16}, kRGBA_8888_SkColorType,
    //                                             kOpaque_SkAlphaType);

    // std::unique_ptr<Recorder> recorder = context->makeRecorder();
    // sk_sp<SkSurface> surface = context->fSurface;//SkSurfaces::RenderTarget(recorder.get(), surfaceInfo);

    // ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    // ResourceCache* resourceCache = resourceProvider->resourceCache();
    // const SharedContext* sharedContext = resourceProvider->sharedContext();



    // Create a Paint which does not require a dst read.
    // SkPaint simplePaint;
    // static const SkBlendMode simpleBlendMode = SkBlendMode::kSrc;
    // static const Coverage coverage = Coverage::kNone;
    // paint.setBlendMode(simpleBlendMode);
    // REPORTER_ASSERT(!IsDstReadRequired(sharedContext->caps(), simpleBlendMode, coverage));
    // surface->getCanvas()
    // canvas->drawColor(SK_ColorBLUE, simpleBlendMode);

    // Create a Paint which we expect to require a dst read.
    // SkPaint dstReadPaint;
    // static const SkBlendMode dstReadBlendMode = SkBlendMode::kLighten;
    // paint.setBlendMode(dstReadBlendMode);
    // REPORTER_ASSERT(IsDstReadRequired(sharedContext->caps(), dstReadBlendMode, coverage));


    // Perform a simple draw which does not require a dst read to a surface that would technically
    // support doing so.

    // Perform a draw which *does* require a dst read to the same surface. This should not add
    // any additional renderpasses, pipelines, etc. since we have optimized the Vulkan backend to
    // be able to reuse existing RPs, pipelines.

    // // Clear entire surface to red
    // SkCanvas* surfaceCanvas = surface->getCanvas();
    // surfaceCanvas->clear(SkColors::kRed);
    // std::unique_ptr<Recording> surfaceRecording = surfaceRecorder->snap();
    // // Flush the clearing
    // context->insertRecording({surfaceRecording.get()});
}

}  // namespace skgpu::graphite
