/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/DispatchGroup.h"
#endif

namespace skgpu::graphite {

PathAtlas::PathAtlas(uint32_t width, uint32_t height) : fRectanizer(width, height) {}

PathAtlas::~PathAtlas() = default;

bool PathAtlas::findRegion(float maskWidth, float maskHeight) {
    // Add a 1 pixel-wide border around the shape bounds when allocating the atlas slot.
    return fRectanizer.findRect(maskWidth + 2, maskHeight + 2);
}

bool PathAtlas::addShape(Recorder* recorder,
                         float deviceOffsetX,
                         float deviceOffsetY,
                         const Shape& shape,
                         const Transform& localToDevice,
                         const SkStrokeRec& style,
                         Rect* out) {
    SkASSERT(out);

    if (!fTexture) {
        // TODO(chromium:1856): Dawn does not support the "storage binding" usage for the R8Unorm
        // texture format. This means that we will have to use RGBA8 until Dawn provides an optional
        // feature.
        fTexture = TextureProxy::MakeStorage(
                recorder->priv().caps(),
                SkISize::Make(int32_t(this->width()), int32_t(this->height())),
                kAlpha_8_SkColorType,
                skgpu::Budgeted::kYes);
        if (!fTexture) {
            return false;
        }
    }

    SkIRect atlasBounds;
    if (!fRectanizer.insertFoundRect(&atlasBounds)) {
        return false;
    }

    *out = Rect(atlasBounds.left(), atlasBounds.top(), atlasBounds.right(), atlasBounds.bottom());
    this->onAddShape(shape, localToDevice, *out, deviceOffsetX, deviceOffsetY, style);

    return true;
}

void PathAtlas::reset() {
    fRectanizer.reset();
    this->onReset();
}

#ifdef SK_ENABLE_VELLO_SHADERS
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
// TODO: This is the maximum target dimension that vello can handle today
constexpr uint32_t kComputeAtlasDim = 4096;

}  // namespace

ComputePathAtlas::ComputePathAtlas() : PathAtlas(kComputeAtlasDim, kComputeAtlasDim) {}

std::unique_ptr<DispatchGroup> ComputePathAtlas::recordDispatches(Recorder* recorder) const {
    if (!this->texture()) {
        return nullptr;
    }

    SkASSERT(recorder);
    return recorder->priv().rendererProvider()->velloRenderer()->renderScene(
            {this->width(), this->height(), SkColors::kBlack},
            fScene,
            sk_ref_sp(this->texture()),
            recorder);
}

void ComputePathAtlas::onAddShape(const Shape& shape,
                                  const Transform& localToDevice,
                                  const Rect& atlasBounds,
                                  float deviceOffsetX,
                                  float deviceOffsetY,
                                  const SkStrokeRec& style) {
    // TODO: The compute renderer doesn't support perspective yet. We assume that the path has been
    // appropriately transformed in that case.
    SkASSERT(localToDevice.type() != Transform::Type::kProjection);

    // TODO(b/283876964): Apply clips here. Initially we'll need to encode the clip stack repeatedly
    // for each shape since the full vello renderer treats clips and their affected draws as a
    // single shape hierarchy in the same scene coordinate space. For coverage masks we want each
    // mask to be transformed to its atlas allocation coordinates and for the clip to be applied
    // with a translation relative to the atlas slot.
    //
    // Repeatedly encoding the clip stack should be relatively cheap (depending on how deep the
    // clips get) however it is wasteful both in terms of time and memory. If this proves to hurt
    // performance, future work will explore building an atlas-oriented element processing stage
    // that applies the atlas-relative translation while evaluating the stack monoid on the GPU.

    // TODO(b/283876964): Aside from clip shapes, we need to clip the shape to the viewport bounds.
    // This is contained in the bounds obtained from ClipStack which should be used here.

    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds`, accounting for the 1
    // pixel-wide border we added earlier, so that the shape is correctly centered.
    SkM44 atlasMatrix = localToDevice.matrix();
    atlasMatrix.postTranslate(atlasBounds.x() + 1 - deviceOffsetX,
                              atlasBounds.y() + 1 - deviceOffsetY);
    Transform atlasTransform(atlasMatrix);
    SkPath devicePath = shape.asPath();

    // For stroke-and-fill, draw two masks into the same atlas slot: one for the stroke and one for
    // the fill.
    SkStrokeRec::Style styleType = style.getStyle();
    if (styleType == SkStrokeRec::kStroke_Style || styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidStroke(devicePath, SkColors::kRed, style.getWidth(), atlasTransform);
    }
    if (styleType == SkStrokeRec::kFill_Style || styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidFill(devicePath, SkColors::kRed, shape.fillType(), atlasTransform);
    }
}

void ComputePathAtlas::onReset() { fScene.reset(); }

#endif  // SK_ENABLE_VELLO_SHADERS

}  // namespace skgpu::graphite
