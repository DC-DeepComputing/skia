/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PathAtlas_DEFINED
#define skgpu_graphite_PathAtlas_DEFINED

#include "include/core/SkStrokeRec.h"
#include "src/gpu/RectanizerSkyline.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/VelloRenderer.h"
#endif

#include <memory>

namespace skgpu::graphite {

class Recorder;
class Rect;
class Shape;
class TextureProxy;
class Transform;

/**
 * PathAtlas manages one or more atlas textures that store coverage masks for path rendering.
 *
 * The contents of a PathAtlas are intended to be transient: atlas regions are considered valid only
 * for the scope of the render passes that sample them. Unlike DrawAtlas, PathAtlas does not support
 * partial eviction and reuse of subregions. Once an atlas texture is filled up, all of its
 * sub-allocations must be invalidated before it can be reused.
 *
 * PathAtlas does not prescribe how atlas contents get uploaded to the GPU. The specific task
 * mechanism is defined by subclasses.
 */
class PathAtlas {
public:
    PathAtlas(uint32_t width, uint32_t height);
    virtual ~PathAtlas();

    /**
     * `findRegion()` searches the atlas for a slot that can fit a coverage mask for a shape with
     * the given bounds in device coordinates. The bounds of the atlas entry is laid out with a 1
     * pixel outset from the given dimensions.
     *
     * Returns false if the shape cannot fit in the atlas. Otherwise, remembers the requested atlas
     * position without allocating the found region. Hence this method does not reduce the available
     * atlas space.
     *
     * The caller can claim the found region and submit a mask shape to be drawn into it by calling
     * `addShape()`
     */
    bool findRegion(float maskWidth, float maskHeight);

    /**
     * Submits a coverage mask shape to be drawn into an atlas region that was located via a
     * preceding call to `findRegion()`. This method effectively claims the found region and returns
     * its boundaries (corresponding to the atlas pixel coordinates) in `outAtlasBounds`.
     *
     * `shape` will be drawn after applying the linear components (scale, rotation, skew) of the
     * provided `localToDevice` transform and the rules in `style`. `deviceOffsetX` and
     * `deviceOffsetY` should match the translation present in `localToDevice`.
     *
     * `addShape()` schedules the shape to be drawn but when and how the rendering happens is
     * specified by the subclass implementation.
     *
     * The stroke-and-fill style is drawn as a single combined coverage mask containing the stroke
     * and the fill.
     *
     * This method lazily creates a TextureProxy that can be referenced by tasks that want to sample
     * the atlas.
     *
     * Returns false if an atlas region was not successfully designated in a preceding call to
     * `findRegion()` or an error occurs while creating the texture.
     */
    bool addShape(Recorder*,
                  float deviceOffsetX,
                  float deviceOffsetY,
                  const Shape& shape,
                  const Transform& localToDevice,
                  const SkStrokeRec& style,
                  Rect* outAtlasBounds);

    // Clear all scheduled atlas draws and free up atlas allocations. After this call the atlas can
    // be considered cleared and available for new shape insertions. However this method does not
    // have any bearing on the contents of the atlas texture itself, which may be in use by GPU
    // commands that are in-flight or yet to be submitted.
    void reset();

    // Returns a pointer to the atlas texture.
    const TextureProxy* texture() const { return fTexture.get(); }

    uint32_t width() const { return static_cast<uint32_t>(fRectanizer.width()); }
    uint32_t height() const { return static_cast<uint32_t>(fRectanizer.height()); }

protected:
    virtual void onAddShape(const Shape&,
                            const Transform& transform,
                            const Rect& atlasBounds,
                            float deviceOffsetX,
                            float deviceOffsetY,
                            const SkStrokeRec&) = 0;
    virtual void onReset() = 0;

private:
    skgpu::RectanizerSkyline fRectanizer;

    // PathAtlas currently supports a single atlas page. The TextureProxy gets created only once and
    // is valid for the lifetime of the PathAtlas. It is possible for the same texture to be bound
    // to multiple DispatchGroups and DrawPasses across flushes. The caller must make sure that any
    // uploads or compute dispatches are scheduled to remain coherent across flushes.
    sk_sp<TextureProxy> fTexture;
};

// NOTE: currently the coverage mask atlas that uses GPU compute uses Vello shaders, so its
// availability is conditioned on SK_ENABLE_VELLO_SHADERS:
#ifdef SK_ENABLE_VELLO_SHADERS

class DispatchGroup;

/**
 * PathAtlas implementation that rasterizes the coverage masks on the GPU using compute shaders.
 *
 * When a new shape gets added, it gets encoded into data streams that will later serve as an input
 * to a series of GPU compute passes. This data is recorded into a DispatchGroup which can be added
 * to a ComputeTask in `recordDispatches()`.
 *
 * After a successful call to `recordDispatches()`, the client is free to call `reset()` and start
 * adding new shapes for a future atlas render.
 */
class ComputePathAtlas final : public PathAtlas {
public:
    ComputePathAtlas();

    // Record the compute dispatches that will draw the atlas contents.
    std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const;

private:
    void onAddShape(const Shape&, const Transform&, const Rect&,
                    float, float, const SkStrokeRec&) override;
    void onReset() override;

    // Contains the encoded scene buffer data that serves as the input to a vello compute pass.
    VelloScene fScene;
};

#endif  // SK_ENABLE_VELLO_SHADERS

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PathAtlas_DEFINED
