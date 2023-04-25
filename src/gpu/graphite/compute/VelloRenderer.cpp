/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/VelloRenderer.h"

#include "include/core/SkPath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"

#include <algorithm>

namespace skgpu::graphite {
namespace {

::rust::Slice<uint8_t> to_slice(void* ptr, size_t size) {
    return {static_cast<uint8_t*>(ptr), size};
}

vello_cpp::Affine to_vello_affine(const SkMatrix& m) {
    // Vello currently doesn't support perspective scaling and the encoding only accepts a 2x3
    // affine transform matrix.
    return {{m.get(0), m.get(3), m.get(1), m.get(4), m.get(2), m.get(5)}};
}

vello_cpp::Color to_vello_color(const SkColor4f& color) {
    SkColor c = color.toSkColor();
    return {
            static_cast<uint8_t>(SkColorGetR(c)),
            static_cast<uint8_t>(SkColorGetG(c)),
            static_cast<uint8_t>(SkColorGetB(c)),
            static_cast<uint8_t>(SkColorGetA(c)),
    };
}

WorkgroupSize to_wg_size(const vello_cpp::WorkgroupSize& src) {
    return WorkgroupSize(src.x, src.y, src.z);
}

bool to_vello_verb(SkPath::Verb verb, vello_cpp::PathVerb* outVerb) {
    switch (verb) {
        case SkPath::kMove_Verb:
            *outVerb = vello_cpp::PathVerb::MoveTo;
            break;
        case SkPath::kLine_Verb:
            *outVerb = vello_cpp::PathVerb::LineTo;
            break;
        case SkPath::kQuad_Verb:
            *outVerb = vello_cpp::PathVerb::QuadTo;
            break;
        case SkPath::kConic_Verb:
        case SkPath::kCubic_Verb:
            *outVerb = vello_cpp::PathVerb::CurveTo;
            break;
        case SkPath::kClose_Verb:
            *outVerb = vello_cpp::PathVerb::Close;
            break;
        case SkPath::kDone_Verb:
        default:
            return false;
    }
    return true;
}

vello_cpp::Fill to_fill_type(SkPathFillType fillType) {
    switch (fillType) {
        case SkPathFillType::kWinding:
            return vello_cpp::Fill::NonZero;
        case SkPathFillType::kEvenOdd:
            return vello_cpp::Fill::EvenOdd;
        default:
            // TODO(b/238756757): vello doesn't define fill types for kInverseWinding and
            // kInverseEvenOdd. This should be updated to support those cases.
            SkDebugf("fill type not supported by vello\n");
            break;
    }
    return vello_cpp::Fill::NonZero;
}

class PathIter : public vello_cpp::PathIterator {
public:
    explicit PathIter(const SkPath& path) : fIter(path, /*forceClose=*/false) {}

    bool next_element(vello_cpp::PathElement* outElem) override {
        SkASSERT(outElem);

        SkPoint points[4];
        SkPath::Verb verb = fIter.next(points);
        if (!to_vello_verb(verb, &outElem->verb)) {
            return false;
        }

        for (size_t i = 0; i < std::size(points); ++i) {
            const SkPoint& p = points[i];
            outElem->points[i] = {p.x(), p.y()};
        }

        return true;
    }

private:
    SkPath::Iter fIter;
};

}  // namespace

VelloScene::VelloScene() : fEncoding(vello_cpp::new_encoding()) {}

void VelloScene::solidFill(const SkPath& shape,
                           const SkColor4f& fillColor,
                           const SkPathFillType fillType,
                           const SkMatrix& t) {
    PathIter iter(shape);
    fEncoding->fill(to_fill_type(fillType),
                    to_vello_affine(t),
                    {vello_cpp::BrushKind::Solid, {to_vello_color(fillColor)}},
                    iter);
    fEmpty = false;
}

void VelloScene::solidStroke(const SkPath& shape,
                             const SkColor4f& fillColor,
                             const StrokeStyle& style,
                             const SkMatrix& t) {
    // Vello currently only supports round stroke styles
    PathIter iter(shape);
    fEncoding->stroke({style.width()},
                      to_vello_affine(t),
                      {vello_cpp::BrushKind::Solid, {to_vello_color(fillColor)}},
                      iter);
    fEmpty = false;
}

void VelloScene::pushClipLayer(const SkPath& shape, const SkMatrix& t) {
    PathIter iter(shape);
    fEncoding->begin_clip(to_vello_affine(t), iter);
    fLayers++;
}

void VelloScene::popClipLayer() {
    SkASSERT(fLayers > 0);
    fEncoding->end_clip();
    fLayers--;
}

VelloRenderer::VelloRenderer(const Caps* caps) {
    fGradientImage = TextureProxy::Make(caps,
                                        {1, 1},
                                        kRGBA_8888_SkColorType,
                                        skgpu::Mipmapped::kNo,
                                        skgpu::Protected::kNo,
                                        skgpu::Renderable::kNo,
                                        skgpu::Budgeted::kYes);
    fImageAtlas = TextureProxy::Make(caps,
                                     {1, 1},
                                     kRGBA_8888_SkColorType,
                                     skgpu::Mipmapped::kNo,
                                     skgpu::Protected::kNo,
                                     skgpu::Renderable::kNo,
                                     skgpu::Budgeted::kYes);
}

VelloRenderer::~VelloRenderer() = default;

std::unique_ptr<DispatchGroup> VelloRenderer::renderScene(const RenderParams& params,
                                                          const VelloScene& scene,
                                                          sk_sp<TextureProxy> target,
                                                          Recorder* recorder) const {
    SkASSERT(target);

    if (scene.fEmpty) {
        return nullptr;
    }

    if (params.fWidth == 0 || params.fHeight == 0) {
        return nullptr;
    }

    // TODO: validate that the pixel format is kRGBA_8888_SkColorType.
    // Clamp the draw region to the target texture dimensions.
    const SkISize dims = target->dimensions();
    if (dims.isEmpty() || dims.fWidth < 0 || dims.fHeight < 0) {
        SKGPU_LOG_W("VelloRenderer: cannot render to an empty target");
        return nullptr;
    }

    SkASSERT(scene.fLayers == 0);  // Begin/end clips must be matched.
    auto config = scene.fEncoding->prepare_render(
            std::min(params.fWidth, static_cast<uint32_t>(dims.fWidth)),
            std::min(params.fHeight, static_cast<uint32_t>(dims.fHeight)),
            to_vello_color(params.fBaseColor));
    auto dispatchInfo = config->workgroup_counts();
    auto bufferSizes = config->buffer_sizes();

    // This is a placeholder instance as the DrawParams are currently unused.
    // TODO: Perhaps there should be an alternate DispatchGroup::appendStep overload that
    // doesn't use a DrawParams. VelloRenderer does not perform any per-draw computation for each
    // ComputeStep and instead calculates all of its buffer and global dispatch sizes up front based
    // on the scene encoding. So the DrawParams parameter is not needed.
    DrawParams placeholder(Transform::Identity(), {}, {}, DrawOrder({}), nullptr);
    DispatchGroup::Builder builder(recorder);

    // In total there are 25 resources that are used across the full pipeline stages. The sizes of
    // these resources depend on the encoded scene. We allocate all of them and assign them
    // directly to the builder here instead of delegating the logic to the ComputeSteps.
    DrawBufferManager* bufMgr = recorder->priv().drawBufferManager();

    size_t uboSize = config->config_uniform_buffer_size();
    auto [uboPtr, configBuf] = bufMgr->getMappedUniform(uboSize);
    if (!config->write_config_uniform_buffer(to_slice(uboPtr, uboSize))) {
        return nullptr;
    }

    size_t sceneSize = config->scene_buffer_size();
    auto [scenePtr, sceneBuf] = bufMgr->getMappedStorage(sceneSize);
    if (!config->write_scene_buffer(to_slice(scenePtr, sceneSize))) {
        return nullptr;
    }

    builder.assignSharedBuffer(configBuf, kVelloSlot_ConfigUniform);
    builder.assignSharedBuffer(sceneBuf, kVelloSlot_Scene);

    // path_reduce
    auto pathtagReduceOutput = bufMgr->getStorage(bufferSizes.path_reduced);
    auto tagmonoid = bufMgr->getStorage(bufferSizes.path_monoids);
    builder.assignSharedBuffer(pathtagReduceOutput, kVelloSlot_PathtagReduceOutput);
    builder.assignSharedBuffer(tagmonoid, kVelloSlot_TagMonoid);
    builder.appendStep(&fPathtagReduce, placeholder, 0, to_wg_size(dispatchInfo.path_reduce));

    // If the input is too large to be fully processed by a single workgroup then a second reduce
    // step and two scan steps are necessary. Otherwise one reduce+scan pair is sufficient.
    //
    // In either case, the result is `tagmonoids`.
    if (dispatchInfo.use_large_path_scan) {
        builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.path_reduced2),
                                   kVelloSlot_LargePathtagReduceSecondPassOutput);
        builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.path_reduced_scan),
                                   kVelloSlot_LargePathtagScanFirstPassOutput);
        builder.appendStep(&fPathtagReduce2, placeholder, 0, to_wg_size(dispatchInfo.path_reduce2));
        builder.appendStep(&fPathtagScan1, placeholder, 0, to_wg_size(dispatchInfo.path_scan1));
        builder.appendStep(&fPathtagScanLarge, placeholder, 0, to_wg_size(dispatchInfo.path_scan));
    } else {
        builder.appendStep(&fPathtagScanSmall, placeholder, 0, to_wg_size(dispatchInfo.path_scan));
    }

    // bbox_clear
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.path_bboxes), kVelloSlot_PathBBoxes);
    builder.appendStep(&fBboxClear, placeholder, 0, to_wg_size(dispatchInfo.bbox_clear));

    // pathseg
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.cubics), kVelloSlot_Cubics);
    builder.appendStep(&fPathseg, placeholder, 0, to_wg_size(dispatchInfo.path_seg));

    // draw_reduce
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.draw_reduced),
                               kVelloSlot_DrawReduceOutput);
    builder.appendStep(&fDrawReduce, placeholder, 0, to_wg_size(dispatchInfo.draw_reduce));

    // draw_leaf
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.draw_monoids), kVelloSlot_DrawMonoid);
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.bin_data), kVelloSlot_InfoBinData);
    // A clip input buffer must still get bound even if the encoding doesn't contain any clips
    builder.assignSharedBuffer(bufMgr->getStorage(std::max(1u, bufferSizes.clip_inps)),
                               kVelloSlot_ClipInput);
    builder.appendStep(&fDrawLeaf, placeholder, 0, to_wg_size(dispatchInfo.draw_leaf));

    // clip_reduce, clip_leaf
    // The clip bbox buffer is always an input to the binning stage, even when the encoding doesn't
    // contain any clips
    builder.assignSharedBuffer(bufMgr->getStorage(std::max(1u, bufferSizes.clip_bboxes)),
                               kVelloSlot_ClipBBoxes);
    WorkgroupSize clipReduceWgCount = to_wg_size(dispatchInfo.clip_reduce);
    WorkgroupSize clipLeafWgCount = to_wg_size(dispatchInfo.clip_leaf);
    bool doClipReduce = clipReduceWgCount.scalarSize() > 0u;
    bool doClipLeaf = clipLeafWgCount.scalarSize() > 0u;
    if (doClipReduce || doClipLeaf) {
        builder.assignSharedBuffer(bufMgr->getStorage(std::max(1u, bufferSizes.clip_bics)),
                                   kVelloSlot_ClipBicyclic);
        builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.clip_els),
                                   kVelloSlot_ClipElement);
        if (doClipReduce) {
            builder.appendStep(&fClipReduce, placeholder, 0, clipReduceWgCount);
        }
        if (doClipLeaf) {
            builder.appendStep(&fClipLeaf, placeholder, 0, clipLeafWgCount);
        }
    }

    // binning
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.draw_bboxes), kVelloSlot_DrawBBoxes);
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.bump_alloc, ClearBuffer::kYes),
                               kVelloSlot_BumpAlloc);
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.bin_headers), kVelloSlot_BinHeader);
    builder.appendStep(&fBinning, placeholder, 0, to_wg_size(dispatchInfo.binning));

    // tile_alloc
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.paths), kVelloSlot_Path);
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.tiles), kVelloSlot_Tile);
    builder.appendStep(&fTileAlloc, placeholder, 0, to_wg_size(dispatchInfo.tile_alloc));

    // path_coarse
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.segments), kVelloSlot_Segments);
    builder.appendStep(&fPathCoarseFull, placeholder, 0, to_wg_size(dispatchInfo.path_coarse));

    // backdrop
    builder.appendStep(&fBackdropDyn, placeholder, 0, to_wg_size(dispatchInfo.backdrop));

    // coarse
    builder.assignSharedBuffer(bufMgr->getStorage(bufferSizes.ptcl), kVelloSlot_PTCL);
    builder.appendStep(&fCoarse, placeholder, 0, to_wg_size(dispatchInfo.coarse));

    // fine
    builder.assignSharedTexture(fImageAtlas, kVelloSlot_ImageAtlas);
    builder.assignSharedTexture(fGradientImage, kVelloSlot_GradientImage);
    builder.assignSharedTexture(std::move(target), kVelloSlot_OutputImage);
    builder.appendStep(&fFine, placeholder, 0, to_wg_size(dispatchInfo.fine));

    return builder.finalize();
}

}  // namespace skgpu::graphite
