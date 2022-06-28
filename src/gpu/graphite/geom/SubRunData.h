/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_SubRunData_DEFINED
#define skgpu_graphite_geom_SubRunData_DEFINED

#include "src/gpu/graphite/geom/Rect.h"

namespace sktext::gpu { class AtlasSubRun; }

namespace skgpu::graphite {

/**
 * SubRunData contains all the data we need to render AtlasSubRuns
 */
class SubRunData {
public:
    SubRunData() = delete;
    SubRunData(const SubRunData& subRun)
        : fSubRun(subRun.fSubRun)
        , fSupportDataKeepAlive(subRun.fSupportDataKeepAlive)
        , fBounds(subRun.fBounds)
        , fStartGlyphIndex(subRun.fStartGlyphIndex)
        , fGlyphCount(subRun.fGlyphCount) {}
    SubRunData(SubRunData&&) = delete;

    SubRunData(const sktext::gpu::AtlasSubRun* subRun,
               const sk_sp<SkRefCnt>&& supportDataKeepAlive,
               Rect deviceBounds, int startGlyphIndex, int glyphCount)
        : fSubRun(subRun)
        , fSupportDataKeepAlive(std::move(supportDataKeepAlive))
        , fBounds(deviceBounds)
        , fStartGlyphIndex(startGlyphIndex)
        , fGlyphCount(glyphCount) {}

    ~SubRunData() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for SubRunData.
    SubRunData& operator=(SubRunData&&) = delete;
    SubRunData& operator=(const SubRunData& that) = default;

    // True if the given bounding box is completely inside the shape, if it's conservatively treated
    // as a filled, closed shape.
    bool conservativeContains(const Rect& rect) const {
        return false;
    }
    bool conservativeContains(skvx::float2 point) const {
        return false;
    }

    // The bounding box of the subrun data.
    Rect bounds() const { return fBounds; }

    // Access the individual elements of the subrun data.
    const sktext::gpu::AtlasSubRun* subRun() { return fSubRun; }

private:
    const sktext::gpu::AtlasSubRun* fSubRun;
    // Keep the TextBlob or Slug alive until we're done with the Geometry.
    sk_sp<SkRefCnt> fSupportDataKeepAlive;

    Rect fBounds;  // bounds of the data stored in the SubRun
    int fStartGlyphIndex;
    int fGlyphCount;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Shape_DEFINED
