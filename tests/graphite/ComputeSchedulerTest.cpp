/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputeScheduler.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/RecorderOptionsPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/image/SkImage_Base.h"
#include "tools/graphite/GraphiteTestContext.h"

#include <cstdint>
#include <cstdlib>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

constexpr int kDim = 8;

SkImageInfo test_info() {
    return SkImageInfo::Make(kDim, kDim, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

// Writes `src + addColor` to `dst`, using the slots that ComputeScheduler assigns. One workgroup
// covers the whole kDim x kDim texture.
class AddColorStep final : public ComputeStep {
public:
    AddColorStep() : ComputeStep(
            /*name=*/"TestComputeSchedulerAddColor",
            /*localDispatchSize=*/{kDim, kDim, 1},
            /*resources=*/{{
                {
                    /*type=*/ResourceType::kReadOnlyTexture,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/ComputeScheduler::kSourceSlot,
                    /*sksl=*/"src",
                },
                {
                    /*type=*/ResourceType::kWriteOnlyStorageTexture,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kNone,
                    /*slot=*/ComputeScheduler::kOutputSlot,
                    /*sksl=*/"dst",
                },
                {
                    /*type=*/ResourceType::kUniformBuffer,
                    /*flow=*/DataFlow::kShared,
                    /*policy=*/ResourcePolicy::kMapped,
                    /*slot=*/ComputeScheduler::kParamsSlot,
                    /*sksl=*/"params { float4 addColor; }",
                },
            }}) {}
    ~AddColorStep() override = default;

    std::string computeSkSL() const override {
        return R"(
            void main() {
                half4 color = textureRead(src, sk_LocalInvocationID.xy);
                textureWrite(dst, sk_LocalInvocationID.xy, saturate(color + half4(addColor)));
            }
        )";
    }

    // Only called to validate the format of the assigned output.
    std::tuple<SkISize, SkColorType> calculateTextureParameters(
            int index, const ResourceDesc&) const override {
        SkASSERT(index == 1);
        return {{kDim, kDim}, kRGBA_8888_SkColorType};
    }
};

std::unique_ptr<Recorder> make_recorder(Context* context, size_t batchBudget) {
    RecorderOptionsPriv optionsPriv;
    optionsPriv.fComputeBatchBudget = batchBudget;
    RecorderOptions options;
    options.fRecorderOptionsPriv = &optionsPriv;
    return context->makeRecorder(options);
}

const Image_Base* as_graphite(const SkImage* image) {
    return static_cast<const Image_Base*>(as_IB(image));
}

const TextureProxy* proxy_of(const SkImage* image) {
    return as_graphite(image)->textureProxyViews()[0].proxy();
}

// Queues an AddColorStep dispatch that reads `src`, and returns the image of its output.
sk_sp<SkImage> enqueue_add(Recorder* recorder,
                           const AddColorStep& step,
                           const SkImage* src,
                           SkColor addColor) {
    const Caps* caps = recorder->priv().caps();
    sk_sp<TextureProxy> output =
            TextureProxy::Make(caps,
                               recorder->priv().resourceProvider(),
                               {kDim, kDim},
                               caps->getDefaultStorageTextureInfo(kRGBA_8888_SkColorType),
                               skgpu::Budgeted::kYes,
                               "ComputeSchedulerTestOutput");
    if (!output) {
        return nullptr;
    }

    const SkColor4f color = SkColor4f::FromColor(addColor);
    auto [writer, params, _] =
            recorder->priv().drawBufferManager()->getMappedUniformBuffer(sizeof(color),
                                                                         /*headroom=*/0);
    if (!params) {
        return nullptr;
    }
    writer.write(color.vec(), sizeof(color));

    ComputeScheduler::Dispatch dispatch;
    dispatch.fStep = &step;
    dispatch.fSource = as_graphite(src);
    dispatch.fOutput = TextureProxyView(std::move(output));
    dispatch.fOutputColorInfo = test_info().colorInfo();
    dispatch.fParams = params;
    dispatch.fGlobalSize = WorkgroupSize(1, 1, 1);
    return recorder->priv().computeScheduler()->enqueue(dispatch);
}

// Returns a client-owned surface cleared to `color`, and an image that's linked to it.
std::pair<sk_sp<SkSurface>, sk_sp<SkImage>> make_source(Recorder* recorder, SkColor color) {
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, test_info());
    if (!surface) {
        return {};
    }
    surface->getCanvas()->clear(color);
    sk_sp<SkImage> image = SkSurfaces::AsImage(surface);
    return {std::move(surface), std::move(image)};
}

// Returns a client-owned surface that `image` has been drawn into.
sk_sp<SkSurface> draw_into_surface(Recorder* recorder, const SkImage* image) {
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, test_info());
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(SK_ColorTRANSPARENT);
    surface->getCanvas()->drawImage(image, 0, 0);
    return surface;
}

// Reads back `surface`, which snaps and inserts the Recorder's pending work, and checks that every
// pixel is `expected` (within one unit per channel).
void check_surface(skiatest::Reporter* reporter,
                   SkSurface* surface,
                   SkColor expected,
                   const char* label) {
    SkBitmap bitmap;
    bitmap.allocPixels(test_info());
    if (!surface || !surface->readPixels(bitmap, 0, 0)) {
        ERRORF(reporter, "%s: failed to read pixels", label);
        return;
    }
    for (int y = 0; y < kDim; ++y) {
        for (int x = 0; x < kDim; ++x) {
            const SkColor found = bitmap.getColor(x, y);
            const bool matches = std::abs(int(SkColorGetA(found)) - int(SkColorGetA(expected))) <= 1 &&
                                 std::abs(int(SkColorGetR(found)) - int(SkColorGetR(expected))) <= 1 &&
                                 std::abs(int(SkColorGetG(found)) - int(SkColorGetG(expected))) <= 1 &&
                                 std::abs(int(SkColorGetB(found)) - int(SkColorGetB(expected))) <= 1;
            if (!matches) {
                ERRORF(reporter, "%s: at (%d, %d), expected 0x%08x, found 0x%08x",
                       label, x, y, expected, found);
                return;
            }
        }
    }
}

constexpr SkColor kSrcColor = SkColorSetARGB(255, 64, 0, 0);
constexpr SkColor kAddGreen = SkColorSetARGB(0, 0, 64, 0);
constexpr SkColor kAddBlue = SkColorSetARGB(0, 0, 0, 64);
constexpr SkColor kAddNothing = SkColorSetARGB(0, 0, 0, 0);

bool is_compute_supported_context_type(skiatest::GpuContextType ctxType) {
    return skiatest::IsDawnContextType(ctxType) ||
           skiatest::IsMetalContextType(ctxType) ||
           skiatest::IsVulkanContextType(ctxType);
}

}  // namespace

#define DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(name, reporter, graphite_context, test_context) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name,                                                       \
                                   is_compute_supported_context_type,                          \
                                   reporter,                                                   \
                                   graphite_context,                                           \
                                   test_context,                                               \
                                   CtsEnforcement::kNever)

// Dispatches stay queued while their outputs are only recorded as reads, and are drained as one
// batch when a reader is flushed (T1). A dispatch that reads an output from the same batch executes
// after the dispatch that writes it.
DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(ComputeScheduler_BatchUntilRead, reporter, context,
                                       testContext) {
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = make_recorder(context, SIZE_MAX);
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    AddColorStep step;

    auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
    sk_sp<SkImage> green = enqueue_add(recorder.get(), step, src.get(), kAddGreen);
    sk_sp<SkImage> greenAndBlue = enqueue_add(recorder.get(), step, green.get(), kAddBlue);
    if (!green || !greenAndBlue) {
        ERRORF(reporter, "Failed to queue dispatches");
        return;
    }
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 2);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 0);

    sk_sp<SkSurface> dst = draw_into_surface(recorder.get(), greenAndBlue.get());
    // Recording a read doesn't drain the batch...
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 2);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 0);

    // ...but flushing the reader does.
    Flush(dst);
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

    check_surface(reporter, dst.get(), SkColorSetARGB(255, 64, 64, 64), "BatchUntilRead");
}

// A budget of 0 drains every dispatch as soon as it's queued. Otherwise, the batch is drained
// before a dispatch would take the scratch sources that it holds over budget (T5).
DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(ComputeScheduler_Budget, reporter, context, testContext) {
    if (context->supportsProtectedContent()) {
        return;
    }
    AddColorStep step;

    {
        std::unique_ptr<Recorder> recorder = make_recorder(context, /*batchBudget=*/0);
        ComputeScheduler* scheduler = recorder->priv().computeScheduler();

        auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
        sk_sp<SkImage> green = enqueue_add(recorder.get(), step, src.get(), kAddGreen);
        REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
        REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

        // Reads an output of the previous batch.
        sk_sp<SkImage> greenAndBlue = enqueue_add(recorder.get(), step, green.get(), kAddBlue);
        REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
        REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 2);
        if (!green || !greenAndBlue) {
            ERRORF(reporter, "Failed to queue dispatches");
            return;
        }

        sk_sp<SkSurface> dst = draw_into_surface(recorder.get(), greenAndBlue.get());
        check_surface(reporter, dst.get(), SkColorSetARGB(255, 64, 64, 64), "Budget 0");
    }

    {
        // Outputs aren't instantiated until their batch is snapped, so they count as scratch
        // sources. Allow one, but not two.
        size_t outputBytes;
        {
            std::unique_ptr<Recorder> recorder = context->makeRecorder();
            const Caps* caps = recorder->priv().caps();
            sk_sp<TextureProxy> probe = TextureProxy::Make(
                    caps, recorder->priv().resourceProvider(), {kDim, kDim},
                    caps->getDefaultStorageTextureInfo(kRGBA_8888_SkColorType),
                    skgpu::Budgeted::kYes, "ComputeSchedulerTestProbe");
            if (!probe) {
                ERRORF(reporter, "Failed to create probe proxy");
                return;
            }
            outputBytes = probe->uninstantiatedGpuMemorySize();
        }
        std::unique_ptr<Recorder> recorder = make_recorder(context, outputBytes * 3 / 2);
        ComputeScheduler* scheduler = recorder->priv().computeScheduler();

        auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
        // The source is instantiated, so it doesn't count.
        sk_sp<SkImage> green = enqueue_add(recorder.get(), step, src.get(), kAddGreen);
        sk_sp<SkImage> blue = enqueue_add(recorder.get(), step, src.get(), kAddBlue);
        REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 2);
        scheduler->drain();
        REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

        sk_sp<SkImage> green1 = enqueue_add(recorder.get(), step, green.get(), kAddNothing);
        // Reading the same source again is free.
        sk_sp<SkImage> green2 = enqueue_add(recorder.get(), step, green.get(), kAddNothing);
        REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 2);
        REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

        // A second scratch source would go over budget.
        sk_sp<SkImage> blue1 = enqueue_add(recorder.get(), step, blue.get(), kAddNothing);
        REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 1);
        REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 2);
        if (!green || !blue || !green1 || !green2 || !blue1) {
            ERRORF(reporter, "Failed to queue dispatches");
            return;
        }

        sk_sp<SkSurface> dstGreen1 = draw_into_surface(recorder.get(), green1.get());
        sk_sp<SkSurface> dstGreen2 = draw_into_surface(recorder.get(), green2.get());
        sk_sp<SkSurface> dstBlue1 = draw_into_surface(recorder.get(), blue1.get());
        check_surface(reporter, dstGreen1.get(), SkColorSetARGB(255, 64, 64, 0), "Budget green1");
        check_surface(reporter, dstGreen2.get(), SkColorSetARGB(255, 64, 64, 0), "Budget green2");
        check_surface(reporter, dstBlue1.get(), SkColorSetARGB(255, 64, 0, 64), "Budget blue1");
    }
}

// A batch is drained before a source is overwritten (T2), so its dispatches read the source's
// contents from when they were queued.
DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(ComputeScheduler_WriteAfterRead, reporter, context,
                                       testContext) {
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = make_recorder(context, SIZE_MAX);
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    AddColorStep step;

    auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
    sk_sp<SkImage> copy = enqueue_add(recorder.get(), step, src.get(), kAddNothing);
    if (!copy) {
        ERRORF(reporter, "Failed to queue dispatch");
        return;
    }

    srcSurface->getCanvas()->clear(SK_ColorBLUE);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 0);
    Flush(srcSurface);
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

    // Flushing a source that has no pending writes doesn't drain the batch.
    sk_sp<SkImage> copy2 = enqueue_add(recorder.get(), step, src.get(), kAddNothing);
    Flush(srcSurface);
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 1);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);

    sk_sp<SkSurface> dst = draw_into_surface(recorder.get(), copy.get());
    sk_sp<SkSurface> dst2 = draw_into_surface(recorder.get(), copy2.get());
    check_surface(reporter, dst.get(), kSrcColor, "WriteAfterRead before");
    check_surface(reporter, dst2.get(), SK_ColorBLUE, "WriteAfterRead after");
    check_surface(reporter, srcSurface.get(), SK_ColorBLUE, "WriteAfterRead source");
}

// A read that's recorded directly to the root task list (e.g. a copy) drains the batch first (T4).
// The output then never becomes a scratch texture, so later reads don't need to be tracked.
DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(ComputeScheduler_ReadWithoutSink, reporter, context,
                                       testContext) {
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = make_recorder(context, SIZE_MAX);
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    AddColorStep step;

    auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
    sk_sp<SkImage> green = enqueue_add(recorder.get(), step, src.get(), kAddGreen);
    if (!green) {
        ERRORF(reporter, "Failed to queue dispatch");
        return;
    }
    // Recorded before the copy, so it's tracked as a pending read of the output.
    sk_sp<SkSurface> dstBefore = draw_into_surface(recorder.get(), green.get());

    sk_sp<SkImage> copy = as_graphite(green.get())->copyImage(recorder.get(),
                                                              SkIRect::MakeWH(kDim, kDim),
                                                              skgpu::Budgeted::kYes,
                                                              skgpu::Mipmapped::kNo,
                                                              SkBackingFit::kExact,
                                                              "ComputeSchedulerTestCopy");
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);
    if (!copy) {
        ERRORF(reporter, "Failed to copy output");
        return;
    }

    // Recorded after the copy unlinked the output, so it isn't tracked.
    sk_sp<SkSurface> dstAfter = draw_into_surface(recorder.get(), green.get());
    sk_sp<SkSurface> dstCopy = draw_into_surface(recorder.get(), copy.get());

    const SkColor expected = SkColorSetARGB(255, 64, 64, 0);
    check_surface(reporter, dstBefore.get(), expected, "ReadWithoutSink before");
    check_surface(reporter, dstAfter.get(), expected, "ReadWithoutSink after");
    check_surface(reporter, dstCopy.get(), expected, "ReadWithoutSink copy");

    // Making a queued output non-budgeted gives it a regular texture right away and also drains.
    sk_sp<SkImage> blue = enqueue_add(recorder.get(), step, src.get(), kAddBlue);
    if (!blue) {
        ERRORF(reporter, "Failed to queue dispatch");
        return;
    }
    sk_sp<Image_Base> nonBudgeted =
            static_cast<Image_Base*>(as_IB(blue.get()))->makeNonBudgeted(recorder.get());
    REPORTER_ASSERT(reporter, nonBudgeted.get() == as_graphite(blue.get()));
    REPORTER_ASSERT(reporter, proxy_of(blue.get())->isInstantiated());
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 2);

    sk_sp<SkSurface> dstBlue = draw_into_surface(recorder.get(), blue.get());
    check_surface(reporter, dstBlue.get(), SkColorSetARGB(255, 64, 0, 64), "ReadWithoutSink NB");
}

// Batches never span Recordings (T3), and an output that's read in a later Recording isn't tracked.
DEF_GRAPHITE_TEST_FOR_COMPUTE_CONTEXTS(ComputeScheduler_ReadInLaterRecording, reporter, context,
                                       testContext) {
    if (context->supportsProtectedContent()) {
        return;
    }
    std::unique_ptr<Recorder> recorder = make_recorder(context, SIZE_MAX);
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    AddColorStep step;

    auto [srcSurface, src] = make_source(recorder.get(), kSrcColor);
    sk_sp<SkImage> green = enqueue_add(recorder.get(), step, src.get(), kAddGreen);
    if (!green) {
        ERRORF(reporter, "Failed to queue dispatch");
        return;
    }

    std::unique_ptr<Recording> recording = recorder->snap();
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);
    if (!recording) {
        ERRORF(reporter, "Failed to snap Recording");
        return;
    }
    // Without readers in its Recording, the output gets a regular texture.
    REPORTER_ASSERT(reporter, proxy_of(green.get())->isInstantiated());
    InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);

    sk_sp<SkSurface> dst = draw_into_surface(recorder.get(), green.get());
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1);
    check_surface(reporter, dst.get(), SkColorSetARGB(255, 64, 64, 0), "ReadInLaterRecording");
}
