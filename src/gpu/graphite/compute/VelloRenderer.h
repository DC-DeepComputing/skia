/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_compute_VelloRenderer_DEFINED
#define skgpu_graphite_compute_VelloRenderer_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStrokeRec.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/compute/VelloComputeSteps.h"
#include "third_party/vello/cpp/vello.h"

#include <memory>

namespace skgpu::graphite {

class Caps;
class DispatchGroup;
class Recorder;
class TextureProxy;
class Transform;

// Encodes Bezier path fills, shapes, and clips. Once populated, this data structure can be used
// with the full compositing and coverage mask generating pipelines. The latter ignores all color
// information.
//
// All color type parameters are expected to be unpremultiplied and in the sRGB color space.
class VelloScene final {
public:
    VelloScene();

    void reset();

    void solidFill(const SkPath&,
                   const SkColor4f&,
                   const SkPathFillType,
                   const Transform& transform);

    void solidStroke(const SkPath&,
                     const SkColor4f&,
                     const SkStrokeRec&,
                     const Transform& transform);

    void pushClipLayer(const SkPath& shape, const Transform& transform);
    void popClipLayer();

private:
    friend class VelloRenderer;

    // Disallow copy
    VelloScene(const VelloScene&) = delete;
    VelloScene& operator=(const VelloScene&) = delete;

    ::rust::Box<::vello_cpp::Encoding> fEncoding;
    SkDEBUGCODE(int fLayers = 0;)
};

// TODO(b/301139790): This is a stop-gap solution to prevent overallocating Vello bump allocation
// arena buffers when vello dispatches get recorded while some are in flight. These buffers are
// transient and GPU-only. It is safe for multiple vello submissions to reuse the same set of
// buffers as long as the submissions don't execute concurrently and the buffers are owned and
// managed by a single Recorder.
//
// Probably a better long term solution is to have a mechanism for a Recording to track and allow
// the reuse of buffer objects across tasks that are serialized.
class VelloReusableBuffers final {
public:
    VelloReusableBuffers() = default;
    ~VelloReusableBuffers() = default;

    BufferView binData(size_t size, Recorder*);
    BufferView tiles(size_t size, Recorder*);
    BufferView segments(size_t size, Recorder*);
    BufferView ptcl(size_t size, Recorder*);

    void reset();

private:
    BufferView allocateIfNeeded(size_t, sk_sp<Buffer>*, Recorder*);

    sk_sp<Buffer> fBinData;
    sk_sp<Buffer> fTiles;
    sk_sp<Buffer> fSegments;
    sk_sp<Buffer> fPtcl;
};

class VelloRenderer final {
public:
    explicit VelloRenderer(const Caps*);
    ~VelloRenderer();

    struct RenderParams {
        // Dimensions of the fine rasterization target
        uint32_t fWidth;
        uint32_t fHeight;

        // The background color used during blending.
        SkColor4f fBaseColor;
    };

    // Run the full pipeline which supports compositing colors with different blend styles. Does
    // nothing if `scene` or target render dimensions are empty. The color type of `target` must
    // be `kRGBA_8888_SkColorType`.
    std::unique_ptr<DispatchGroup> renderScene(const RenderParams&,
                                               const VelloScene&,
                                               sk_sp<TextureProxy> target,
                                               Recorder*,
                                               VelloReusableBuffers*) const;

private:
    // Pipelines
    VelloBackdropStep fBackdrop;
    VelloBackdropDynStep fBackdropDyn;
    VelloBboxClearStep fBboxClear;
    VelloBinningStep fBinning;
    VelloClipLeafStep fClipLeaf;
    VelloClipReduceStep fClipReduce;
    VelloCoarseStep fCoarse;
    VelloDrawLeafStep fDrawLeaf;
    VelloDrawReduceStep fDrawReduce;
    VelloFineStep fFine;
    VelloPathCoarseFullStep fPathCoarseFull;
    VelloPathsegStep fPathseg;
    VelloPathtagReduceStep fPathtagReduce;
    VelloPathtagReduce2Step fPathtagReduce2;
    VelloPathtagScan1Step fPathtagScan1;
    VelloPathtagScanLargeStep fPathtagScanLarge;
    VelloPathtagScanSmallStep fPathtagScanSmall;
    VelloTileAllocStep fTileAlloc;

    // The full renderer uses an image atlas and a gradient ramp texture for image composition and
    // gradient fills, respectively. These are currently unused, so we allocate and reuse two 1x1
    // textures to satisfy the shader bindings.
    //
    // TODO: The contents of these textures will be scene dependent. Re-evaluate if/when we enable
    // gradient fills or images.
    sk_sp<TextureProxy> fGradientImage;
    sk_sp<TextureProxy> fImageAtlas;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_compute_VelloRenderer_DEFINED
