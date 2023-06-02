/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AtlasShape_DEFINED
#define skgpu_graphite_geom_AtlasShape_DEFINED

#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

class PathAtlas;

/**
 * AtlasShape represents a shape for which per-pixel coverage data comes from a transient atlas
 * texture. This excludes font glyphs that are rendered to a persistent atlas, as those are
 * represented by the SubRunData geometry type.
 */
class AtlasShape {
public:
    AtlasShape() = default;
    AtlasShape(const Shape& shape, Rect maskBounds, Rect atlasBounds, const PathAtlas* atlas)
            : fBounds(shape.bounds())
            , fMaskBounds(maskBounds)
            , fAtlasBounds(atlasBounds)
            , fAtlas(atlas)
            , fInverted(shape.inverted()) {}
    AtlasShape(const AtlasShape&) = default;

    ~AtlasShape() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for AtlasShape.
    AtlasShape& operator=(AtlasShape&&) = delete;
    AtlasShape& operator=(const AtlasShape&) = default;

    // The bounds of the underlying shape in local coordinates. This has the same semantics as the
    // `bounds()` method for all Geometry types.
    const Rect& bounds() const { return fBounds; }

    // The bounds of the shape in device coordinates after applying any expansion due to stroke
    // style. For strokes and regular fills this corresponds to the draw bounds. For inverse
    // fills, this represents the masked, zero-coverage region within the effective draw bounds.
    // TODO(b/283876964): When the shape is partially clipped, the atlas mask is going to have a
    // clip applied to it (either viewport or shape). The draw bounds obtained from ClipStack
    // accurately contains the clipped bounds and accurately represents an inversely filled region.
    // For inverse fills and clips, AtlasShape will also store the correct "draw bounds" and the
    // RenderStep will use both of them in combination to correctly sample the texture.
    const Rect& maskBounds() const { return fMaskBounds; }

    // The bounds of the atlas region in atlas-relative coordinates.
    const Rect& atlasBounds() const { return fAtlasBounds; }

    // The atlas that the shape will be rendered to.
    const PathAtlas* atlas() const { return fAtlas; }

    // Whether or not the shape will be painted according to an inverse fill rule.
    bool inverted() const { return fInverted; }

private:
    Rect fBounds;
    Rect fMaskBounds;

    // The bounding region of the atlas plot allocated for this shape in atlas pixel coordinates.
    Rect fAtlasBounds;
    const PathAtlas* fAtlas;
    bool fInverted;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AtlasShape_DEFINED
