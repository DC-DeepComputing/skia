/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StripGenerator_DEFINED
#define skgpu_graphite_sparse_strips_StripGenerator_DEFINED

#include "include/core/SkRect.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <cstdint>

#if defined(GPU_TEST_UTILS)
#include "include/core/SkSize.h"

#include <functional>
#endif

class SkMatrix;
class SkPath;

#if defined(GPU_TEST_UTILS)
class SparseStripsDebugSlide;
#endif

namespace skgpu::graphite {

class Recorder;
class Shape;

class StripGenerator {
public:
    // Marks the generator's current draw as in flight, from before processGeometry() until its
    // EndCaps have been recorded. See AlphaAtlasManager::DrawScope.
    class DrawScope {
    public:
        explicit DrawScope(StripGenerator* generator);

    private:
        AlphaAtlasManager::DrawScope fAtlasScope;
    };

    StripGenerator(int width,
                   int height,
                   const SparseStripConfig::LUTArray& maskLUT,
                   Recorder* recorder);
    ~StripGenerator();

    // Generates the strips for `shape` drawn with `ctm`, restricted to `clip`. The cached strips of
    // an earlier draw of the shape, and their alphas, may be reused instead (see
    // AlphaAtlasManager::findCachedStrips()): those of a draw of the shape within its clip with the
    // same transform up to an integer translation or, if this draw's strips depend on the clip
    // (the shape is clipped or inverse filled), those of a draw with the same transform and clip.
    bool processGeometry(const Shape& shape, const SkMatrix& ctm, const SkIRect& clip);

    // Always generates new strips for `path`, without consulting or populating the strip cache.
    bool processGeometry(const SkPath& path, const SkMatrix& ctm);
    bool processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip);

    const EndCaps& ends() const { return fEnds; }
    const WideTiles& wides() const { return fWides; }

    // Returns true if committing the current draw's overflow alphas requires a flush first.
    bool overflowNeedsFlush() const;
    // Commits the current draw's overflow alphas to the alpha atlas' overflow page. Must be called
    // after processGeometry() and before the EndCaps are recorded.
    bool commitOverflow();

private:
#if defined(GPU_TEST_UTILS)
    friend class ::SparseStripsDebugSlide;

    // Debug-only hook which observes the exact inputs of every processGeometry() call, so that
    // viewer's SparseStripsDebug slide can capture a frame for offline repro. Not thread safe;
    // install and remove it from the thread that records draws. Pass an empty function to
    // uninstall.
    using DebugCaptureFn = std::function<void(const SkPath& path,
                                              const SkMatrix& ctm,
                                              const SkIRect& clip,
                                              SkISize targetSize)>;
    static void SetDebugCaptureCallback(DebugCaptureFn fn);
    // Returns the installed hook, or an empty function if none is installed.
    static const DebugCaptureFn& GetDebugCaptureCallback();
    static DebugCaptureFn& DebugCaptureCallbackStorage();
#endif

    AlphaAtlasManager* atlasManager() const;

    // Resets the generator's strips for a new draw. Returns null if there is no alpha atlas.
    AlphaAtlasManager* beginDraw();

    // Generates the strips for `path` within the non-empty, tile aligned `tileClip` into fEnds and
    // fWides. If any strips were generated, fCulled is set if tiling culled any of the path's
    // geometry.
    bool generateStrips(AlphaAtlasManager* atlasManager,
                        const SkPath& path,
                        const SkMatrix& ctm,
                        const SkIRect& tileClip);

    int fWidth;
    int fHeight;
    const SparseStripConfig::LUTArray& fMaskLUT;
    Recorder* fRecorder;
    EndCaps fEnds;
    WideTiles fWides;
    WindingHistogram fWindingHistogram;
    bool fCulled;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_StripGenerator_DEFINED
