/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/DrawContext.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPixmap.h"
#include "src/core/SkColorData.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePathAtlas.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/task/ComputeTask.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/DrawTask.h"
#include "src/gpu/graphite/task/RenderPassTask.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

namespace skgpu::graphite {

sk_sp<DrawContext> DrawContext::Make(const Caps* caps,
                                     sk_sp<TextureProxy> target,
                                     SkISize deviceSize,
                                     const SkColorInfo& colorInfo,
                                     const SkSurfaceProps& props) {
    if (!target) {
        return nullptr;
    }
    // We don't render to unknown or unpremul alphatypes
    if (colorInfo.alphaType() == kUnknown_SkAlphaType ||
        colorInfo.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    if (!caps->isRenderable(target->textureInfo())) {
        return nullptr;
    }
    if (!caps->areColorTypeAndTextureInfoCompatible(colorInfo.colorType(), target->textureInfo())) {
        return nullptr;
    }

    // Accept an approximate-fit texture, but make sure it's at least as large as the device's
    // logical size.
    // TODO: validate that the alpha type is compatible with the target's info
    SkASSERT(target->isFullyLazy() || (target->dimensions().width() >= deviceSize.width() &&
                                       target->dimensions().height() >= deviceSize.height()));
    SkImageInfo imageInfo = SkImageInfo::Make(deviceSize, colorInfo);
    return sk_sp<DrawContext>(new DrawContext(caps, std::move(target), imageInfo, props));
}

namespace {
DstReadStrategy determine_msaa_dstReadStrategy(DstReadStrategy singleSampledStrategy) {
    // TODO(b/390458117): Vulkan is currently the only backend to utilize
    // DstReadStrategy::kReadFromInput. However, it does not yet support reading from multisampled
    // textures as input attachments. So, we must fall back to using texture copies when
    // multisampling. Once Vulkan supports dst reads for multisampled targets, we can consolidate to
    // using one shared dst read strategy for all targets regardless of sample count.
    return singleSampledStrategy == DstReadStrategy::kReadFromInput ? DstReadStrategy::kTextureCopy
                                                                    : singleSampledStrategy;
}
} // anonymous namespace

DrawContext::DrawContext(const Caps* caps,
                         sk_sp<TextureProxy> target,
                         const SkImageInfo& ii,
                         const SkSurfaceProps& props)
        : fTarget(std::move(target))
        , fImageInfo(ii)
        , fSurfaceProps(props)
        , fSingleSampleDstReadStrategy(caps->getDstReadStrategy())
        , fMSAADstReadStrategy(determine_msaa_dstReadStrategy(fSingleSampleDstReadStrategy))
        , fCurrentDrawTask(sk_make_sp<DrawTask>(fTarget))
        , fPendingDraws(std::make_unique<DrawList>())
        , fPendingUploads(std::make_unique<UploadList>()) {
    // Must determine a valid strategy to use should a dst texture read be required.
    SkASSERT(fSingleSampleDstReadStrategy != DstReadStrategy::kNoneRequired);

    if (!caps->isTexturable(fTarget->textureInfo())) {
        fReadView = {}; // Presumably this DrawContext is rendering into a swap chain
    } else {
        Swizzle swizzle = caps->getReadSwizzle(ii.colorType(), fTarget->textureInfo());
        fReadView = {fTarget, swizzle};
    }
    // TBD - Will probably want DrawLists (and its internal commands) to come from an arena
    // that the DC manages.
}

DrawContext::~DrawContext() = default;

void DrawContext::clear(const SkColor4f& clearColor) {
    this->internalDiscard();
    fPendingDraws->reset(LoadOp::kClear, clearColor);
}

void DrawContext::discard() {
    this->internalDiscard();
    fPendingDraws->reset(LoadOp::kDiscard);
}

void DrawContext::internalDiscard() {
    // Non-loading operations on a fully lazy target can corrupt data beyond the DrawContext's
    // region so should be avoided.
    SkASSERT(!fTarget->isFullyLazy());

    // NOTE: Eventually the current DrawTask should be reset, once there are no longer implicit
    // dependencies on atlas tasks between DrawContexts. When that's resolved, the only tasks in the
    // current DrawTask are those that directly impact the target, which becomes irrelevant with the
    // clear op overwriting it. For now, preserve the previous tasks that might include atlas
    // uploads that are not explicitly shared between DrawContexts.
    if (fComputePathAtlas) {
        fComputePathAtlas->reset();
    }
}

void DrawContext::recordDraw(const Renderer* renderer,
                             const Transform& localToDevice,
                             const Geometry& geometry,
                             const Clip& clip,
                             DrawOrder ordering,
                             UniquePaintParamsID paintID,
                             DstReadStrategy dstReadStrategy,
                             bool dependsOnDst,
                             SkBlendMode finalBlendMode,
                             PipelineDataGatherer* gatherer,
                             const StrokeStyle* stroke) {
    SkASSERT(SkIRect::MakeSize(this->imageInfo().dimensions()).contains(clip.scissor()));
    fPendingDraws->recordDraw(renderer, localToDevice, geometry, clip, ordering, paintID,
                              dstReadStrategy, dependsOnDst, finalBlendMode, gatherer, stroke);
}

bool DrawContext::recordUpload(Recorder* recorder,
                               sk_sp<TextureProxy> targetProxy,
                               const SkColorInfo& srcColorInfo,
                               const SkColorInfo& dstColorInfo,
                               const std::vector<MipLevel>& levels,
                               const SkIRect& dstRect,
                               std::unique_ptr<ConditionalUploadContext> condContext) {
    // Our caller should have clipped to the bounds of the surface already.
    SkASSERT(targetProxy->isFullyLazy() ||
             SkIRect::MakeSize(targetProxy->dimensions()).contains(dstRect));
    return fPendingUploads->recordUpload(recorder,
                                         std::move(targetProxy),
                                         srcColorInfo,
                                         dstColorInfo,
                                         levels,
                                         dstRect,
                                         std::move(condContext));
}

void DrawContext::recordDependency(sk_sp<Task> task) {
    SkASSERT(task);
    // Adding `task` to the current DrawTask directly means that it will execute after any previous
    // dependent tasks and after any previous calls to flush(), but everything else that's being
    // collected on the DrawContext will execute after `task` once the next flush() is performed.
    fCurrentDrawTask->addTask(std::move(task));
}

PathAtlas* DrawContext::getComputePathAtlas(Recorder* recorder) {
    if (!fComputePathAtlas) {
        fComputePathAtlas = recorder->priv().atlasProvider()->createComputePathAtlas(recorder);
    }
    return fComputePathAtlas.get();
}

void DrawContext::flush(Recorder* recorder, SkSpan<const float> gradientData) {
    if (fPendingUploads->size() > 0) {
        TRACE_EVENT_INSTANT1("skia.gpu", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                             "# uploads", fPendingUploads->size());
        fCurrentDrawTask->addTask(UploadTask::Make(fPendingUploads.get()));
        // The UploadTask steals the collected upload instances, automatically resetting this list
        SkASSERT(fPendingUploads->size() == 0);
    }

    // Generate compute dispatches that render into the atlas texture used by pending draws.
    // TODO: Once compute atlas caching is implemented, DrawContext might not hold onto to this
    // at which point a recordDispatch() could be added and it stores a pending dispatches list that
    // much like how uploads are handled. In that case, Device would be responsible for triggering
    // the recording of dispatches, but that may happen naturally in AtlasProvider::recordUploads().
    if (fComputePathAtlas) {
        ComputeTask::DispatchGroupList dispatches;
        if (fComputePathAtlas->recordDispatches(recorder, &dispatches)) {
            // For now this check is valid as all coverage mask draws involve dispatches
            SkASSERT(fPendingDraws->hasCoverageMaskDraws());

            fCurrentDrawTask->addTask(ComputeTask::Make(std::move(dispatches)));
        } // else no pending compute work needed to be recorded

        fComputePathAtlas->reset();
    } // else platform doesn't support compute or atlas was never initialized.

    if (!fPendingDraws->modifiesTarget()) {
        // Nothing will be rasterized to the target that warrants a RenderPassTask, but we preserve
        // any added uploads or compute tasks since those could also affect the target w/o
        // rasterizing anything directly.
        return;
    }

    // Extract certain properties from DrawList relevant for DrawTask construction before
    // relinquishing the pending draw list to the DrawPass constructor.
    SkIRect dstReadPixelBounds = fPendingDraws->dstReadBounds().makeRoundOut().asSkIRect();
    const bool drawsRequireMSAA = fPendingDraws->drawsRequireMSAA();
    const SkEnumBitMask<DepthStencilFlags> dsFlags = fPendingDraws->depthStencilFlags();
    // Determine the optimal dst read strategy for the drawpass given pending draw characteristics
    const DstReadStrategy drawPassDstReadStrategy =
            fPendingDraws->drawsReadDst() ? this->dstReadStrategy(fPendingDraws->drawsRequireMSAA())
                                          : DstReadStrategy::kNoneRequired;

    // Convert the pending draws and load/store ops into a DrawPass that will be executed after
    // the collected uploads and compute dispatches.
    // TODO: At this point, there's only ever one DrawPass in a RenderPassTask to a target. When
    // subpasses are implemented, they will either be collected alongside fPendingDraws or added
    // to the RenderPassTask separately.
    std::unique_ptr<DrawPass> pass = fPendingDraws->snapDrawPass(recorder,
                                                                 fTarget,
                                                                 fImageInfo.dimensions(),
                                                                 gradientData);
    SkASSERT(!fPendingDraws->modifiesTarget()); // Should be drained into `pass`.

    if (pass) {
        SkASSERT(fTarget.get() == pass->target());

        // If any paint used within the DrawPass reads from the dst texture (indicated by nonempty
        // dstReadPixelBounds) and the dstReadStrategy is kTextureCopy, then add a CopyTask.
        sk_sp<TextureProxy> dstCopy;
        if (!dstReadPixelBounds.isEmpty() &&
            drawPassDstReadStrategy == DstReadStrategy::kTextureCopy) {
            TRACE_EVENT_INSTANT0("skia.gpu", "DrawPass requires dst copy",
                                 TRACE_EVENT_SCOPE_THREAD);

            // TODO: Right now this assert is ensuring that the dstCopy will be texturable since it
            // uses the same texture info as fTarget. Ideally, if fTarget were not texturable but
            // still readable, we would perform a fallback to a compatible texturable info. We also
            // should decide whether or not a copy-as-draw fallback is necessary here too. All of
            // this is handled inside Image::Copy() except we would need it to expose the task in
            // order to link it correctly.
            SkASSERT(recorder->priv().caps()->isTexturable(fTarget->textureInfo()));
            // Use approx size for better reuse.
            SkISize dstCopyTextureSize = GetApproxSize(dstReadPixelBounds.size());
            dstCopy = TextureProxy::Make(recorder->priv().caps(),
                                         recorder->priv().resourceProvider(),
                                         dstCopyTextureSize,
                                         fTarget->textureInfo(),
                                         "DstCopyTexture",
                                         skgpu::Budgeted::kYes);
            SkASSERT(dstCopy);

            // Add the copy task to initialize dstCopy before the render pass task.
            fCurrentDrawTask->addTask(CopyTextureToTextureTask::Make(
                    fTarget, dstReadPixelBounds, dstCopy, /*dstPoint=*/{0, 0}));
        }

        const Caps* caps = recorder->priv().caps();
        auto [loadOp, storeOp] = pass->ops();
        auto writeSwizzle = caps->getWriteSwizzle(this->colorInfo().colorType(),
                                                  fTarget->textureInfo());

        RenderPassDesc desc = RenderPassDesc::Make(caps, fTarget->textureInfo(), loadOp, storeOp,
                                                   dsFlags,
                                                   pass->clearColor(),
                                                   drawsRequireMSAA,
                                                   writeSwizzle,
                                                   drawPassDstReadStrategy);

        RenderPassTask::DrawPassList passes;
        passes.emplace_back(std::move(pass));
        fCurrentDrawTask->addTask(RenderPassTask::Make(std::move(passes), desc, fTarget,
                                                       std::move(dstCopy), dstReadPixelBounds));
    }
    // else pass creation failed, DrawPass will have logged why. Don't discard the previously
    // accumulated tasks, however, since they may represent operations on an atlas that other
    // DrawContexts now implicitly depend on.
}

sk_sp<Task> DrawContext::snapDrawTask() {
    if (!fCurrentDrawTask->hasTasks()) {
        return nullptr;
    }

    sk_sp<Task> snappedTask = std::move(fCurrentDrawTask);
    fCurrentDrawTask = sk_make_sp<DrawTask>(fTarget);
    return snappedTask;
}

} // namespace skgpu::graphite
