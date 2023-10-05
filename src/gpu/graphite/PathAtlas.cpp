/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/DispatchGroup.h"
#endif

namespace skgpu::graphite {
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
// TODO: This is the maximum target dimension that vello can handle today
constexpr uint16_t kComputeAtlasDim = 4096;

}  // namespace

PathAtlas::PathAtlas(uint32_t width, uint32_t height) : fWidth(width), fHeight(height) {}

PathAtlas::~PathAtlas() = default;

std::optional<PathAtlas::MaskAndOrigin> PathAtlas::addShape(
        Recorder* recorder,
        const Rect& transformedShapeBounds,
        const Shape& shape,
        const Transform& localToDevice,
        const SkStrokeRec& style) {
    const bool emptyMask = transformedShapeBounds.isEmptyNegativeOrNaN();

    // Round out the shape bounds to preserve any fractional offset so that it is present in the
    // translation that we use when deriving the atlas-space transform later.
    Rect maskBounds = transformedShapeBounds.makeRoundOut();

    // Add an additional one pixel outset as buffer between atlas slots. This prevents sampling from
    // neighboring atlas slots; the CoverageMask renderer also uses the outset to sample zero
    // coverage on inverse fill pixels that fall outside the mask bounds.
    skvx::float2 maskSize  = emptyMask ? skvx::float2{0.f, 0.f} : maskBounds.size();
    skvx::float2 atlasSize = emptyMask ? skvx::float2{0.f, 0.f} : maskSize + 2;

    // It is possible for the transformed shape bounds to be fully clipped out while the draw
    // still produces coverage due to an inverse fill. In this case, don't render any mask;
    // CoverageMaskShapeRenderStep will automatically handle the simple fill. We'll handle this
    // by adding an empty mask but still returning an instantiated MaskAndOrigin.
    SkIPoint16 pos;
    const TextureProxy* atlasProxy = this->addRect(recorder, atlasSize, &pos);
    if (!atlasProxy) {
        return {};
    }

    CoverageMaskShape::MaskInfo maskInfo{/*fTextureOrigin=*/skvx::half2(0),
                                         /*fMaskSize=*/skvx::cast<uint16_t>(maskSize)};
    SkIPoint deviceOrigin{0, 0};
    if (!emptyMask) {
        // Add 1 to the entry's recorded mask position so that the actual mask coverage values are
        // defined with a 1px border.
        skvx::float2 atlasOrigin{(float) pos.x() + 1, (float) pos.y() + 1};
        maskInfo.fTextureOrigin = skvx::cast<uint16_t>(atlasOrigin);

        // The transform applied to shape is 'localToDevice' with a post-translation that maps
        // the top-left corner of 'maskBounds' (e.g. device-space mask origin) to 'atlasOrigin'.
        deviceOrigin.fX = (int) maskBounds.left();
        deviceOrigin.fY = (int) maskBounds.top();
        Transform atlasTransform = localToDevice.postTranslate(atlasOrigin.x() - deviceOrigin.x(),
                                                               atlasOrigin.y() - deviceOrigin.y());

        this->onAddShape(shape, atlasTransform,  Rect::XYWH(atlasOrigin, maskSize), style);

    }

    return std::make_pair(CoverageMaskShape{shape, atlasProxy, localToDevice.inverse(), maskInfo},
                          deviceOrigin);
}

///////////////////////////////////////////////////////////////////////////////////////

ComputePathAtlas::ComputePathAtlas()
    : PathAtlas(kComputeAtlasDim, kComputeAtlasDim)
    , fRectanizer(fWidth, fHeight) {}

bool ComputePathAtlas::initializeTextureIfNeeded(Recorder* recorder) {
    if (!fTexture) {
        SkColorType targetCT = ComputeShaderCoverageMaskTargetFormat(recorder->priv().caps());
        fTexture = recorder->priv().atlasProvider()->getAtlasTexture(recorder,
                                                                     this->width(),
                                                                     this->height(),
                                                                     targetCT,
                                                                     /*requiresStorageUsage=*/true);
    }
    return fTexture != nullptr;
}

const TextureProxy* ComputePathAtlas::addRect(Recorder* recorder,
                                              skvx::float2 atlasSize,
                                              SkIPoint16* pos) {
    if (!this->initializeTextureIfNeeded(recorder)) {
        SKGPU_LOG_E("Failed to instantiate an atlas texture");
        return nullptr;
    }

    // An empty mask always fits, so just return the texture.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(atlasSize)) {
        return fTexture.get();
    }

    if (!fRectanizer.addRect(atlasSize.x(), atlasSize.y(), pos)) {
        return nullptr;
    }

    return fTexture.get();
}

void ComputePathAtlas::reset() {
    fRectanizer.reset();

    this->onReset();
}

#ifdef SK_ENABLE_VELLO_SHADERS

std::unique_ptr<DispatchGroup> VelloComputePathAtlas::recordDispatches(Recorder* recorder) const {
    if (!this->texture()) {
        return nullptr;
    }

    SkASSERT(recorder);
    return recorder->priv().rendererProvider()->velloRenderer()->renderScene(
            {fOccuppiedWidth, fOccuppiedHeight, SkColors::kBlack},
            fScene,
            sk_ref_sp(this->texture()),
            recorder);
}

void VelloComputePathAtlas::onAddShape(const Shape& shape,
                                       const Transform& atlasTransform,
                                       const Rect& atlasBounds,
                                       const SkStrokeRec& style) {
    // TODO: The compute renderer doesn't support perspective yet. We assume that the path has been
    // appropriately transformed in that case.
    SkASSERT(localToDevice.type() != Transform::Type::kProjection);

    // Restrict the render to the occupied area of the atlas.
    fOccuppiedWidth = std::max(fOccuppiedWidth, (uint32_t)atlasBounds.right());
    fOccuppiedHeight = std::max(fOccuppiedHeight, (uint32_t)atlasBounds.bot());

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
    Rect clipBounds(atlasBounds.topLeft(), atlasBounds.botRight());
    SkPath clipRect = SkPath::Rect(clipBounds.asSkRect());
    fScene.pushClipLayer(clipRect, Transform::Identity());

    SkPath devicePath = shape.asPath();

    // For stroke-and-fill, draw two masks into the same atlas slot: one for the stroke and one for
    // the fill.
    SkStrokeRec::Style styleType = style.getStyle();
    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // We need to special-case hairline strokes and strokes with sub-pixel width as Vello
        // draws these with aliasing and the results are barely visible. Draw the stroke with a
        // device-space width of 1 pixel and scale down the alpha by the true width to approximate
        // the sampled area.
        float width = style.getWidth();
        float deviceWidth = width * atlasTransform.maxScaleFactor();
        if (style.isHairlineStyle() || deviceWidth <= 1.0) {
            // Both strokes get 1/2 weight scaled by the theoretical area (1 for hairlines,
            // `deviceWidth` otherwise).
            SkColor4f color = SkColors::kRed;
            color.fR *= style.isHairlineStyle() ? 1.0 : deviceWidth;

            // Transform the stroke's width to its local coordinate space since it'll get drawn with
            // `atlasTransform`.
            float transformedWidth = 1.0f / atlasTransform.maxScaleFactor();
            SkStrokeRec adjustedStyle(style);
            adjustedStyle.setStrokeStyle(transformedWidth);
            fScene.solidStroke(devicePath, color, adjustedStyle, atlasTransform);
        } else {
            fScene.solidStroke(devicePath, SkColors::kRed, style, atlasTransform);
        }
    }
    if (styleType == SkStrokeRec::kFill_Style || styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidFill(devicePath, SkColors::kRed, shape.fillType(), atlasTransform);
    }

    fScene.popClipLayer();
}

#endif  // SK_ENABLE_VELLO_SHADERS

}  // namespace skgpu::graphite
