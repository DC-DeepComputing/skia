/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"

#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/QuadTreeBoundsManager.h"

static DEFINE_string(intersectionTreeFile, "",
                     "svg or skp for the IntersectionTree bench to sniff paths from.");

namespace skgpu::graphite {

class IntersectionTreeBench : public Benchmark {
protected:
    const char* onGetName() final { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() final {
        SkTArray<SkRect> rects;
        this->gatherRects(&rects);
        fRectCount = rects.count();
        fRects = fAlignedAllocator.makeArray<Rect>(fRectCount);
        for (int i = 0; i < fRectCount; ++i) {
            fRects[i] = rects[i];
        }
        fRectBufferA = fAlignedAllocator.makeArray<Rect>(fRectCount);
        fRectBufferB = fAlignedAllocator.makeArray<Rect>(fRectCount);
    }

    virtual void gatherRects(SkTArray<SkRect>* rects) = 0;

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void doBench() {
        Rect* rects = fRects;
        Rect* collided = fRectBufferA;
        int rectCount = fRectCount;
        fNumTrees = 0;
        while (rectCount > 0) {
            IntersectionTree intersectionTree;
            int collidedCount = 0;
            for (int i = 0; i < rectCount; ++i) {
                if (!intersectionTree.add(rects[i])) {
                    collided[collidedCount++] = rects[i];
                }
            }
            std::swap(rects, collided);
            if (collided == fRects) {
                collided = fRectBufferB;
            }
            rectCount = collidedCount;
            ++fNumTrees;
        }
    }

    SkString fName;
    SkArenaAlloc fAlignedAllocator{0};
    int fRectCount;
    Rect* fRects;
    Rect* fRectBufferA;
    Rect* fRectBufferB;
    int fNumTrees = 0;
};

class RandomIntersectionBench : public IntersectionTreeBench {
public:
    RandomIntersectionBench(int numRandomRects) : fNumRandomRects(numRandomRects) {
        fName.printf("IntersectionTree_%i", numRandomRects);
    }

private:
    void gatherRects(SkTArray<SkRect>* rects) override {
        SkRandom rand;
        for (int i = 0; i < fNumRandomRects; ++i) {
            rects->push_back(SkRect::MakeXYWH(rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 70),
                                              rand.nextRangeF(0, 70)));
        }
    }

    const int fNumRandomRects;
};

class FileIntersectionBench : public IntersectionTreeBench {
public:
    FileIntersectionBench() {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        const char* filename = strrchr(FLAGS_intersectionTreeFile[0], '/');
        if (filename) {
            ++filename;
        } else {
            filename = FLAGS_intersectionTreeFile[0];
        }
        fName.printf("IntersectionTree_file_%s", filename);
    }

private:
    bool isSuitableFor(Backend backend) final {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return false;
        }
        return IntersectionTreeBench::isSuitableFor(backend);
    }

    void gatherRects(SkTArray<SkRect>* rects) override {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        ToolUtils::sniff_paths(FLAGS_intersectionTreeFile[0], [&](const SkMatrix& matrix,
                                                                  const SkPath& path,
                                                                  const SkPaint& paint) {
            if (paint.getStyle() == SkPaint::kStroke_Style) {
                return;  // Goes to stroker.
            }
            if (path.isConvex()) {
                return;  // Goes to convex renderer.
            }
            int numVerbs = path.countVerbs();
            SkRect drawBounds = matrix.mapRect(path.getBounds());
            float gpuFragmentWork = drawBounds.height() * drawBounds.width();
            float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
            constexpr static float kCpuWeight = 512;
            constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
            if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
                return;  // Goes to inner triangulator.
            }
            rects->push_back(drawBounds);
        });
        SkDebugf(">> Found %i stencil/cover paths in %s <<\n",
                 rects->count(), FLAGS_intersectionTreeFile[0]);
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        SkDebugf(">> Reordered %s into %i different stencil/cover draws <<\n",
                 FLAGS_intersectionTreeFile[0], fNumTrees);
    }
};

class BoundsManagerBench : public Benchmark {
public:
    BoundsManagerBench(std::unique_ptr<BoundsManager> manager, const char* managerName, int numRandomRects)
            : fManager(std::move(manager))
            , fRectCount(numRandomRects) {
        fName.printf("BoundsManager_%i_%s", numRandomRects, managerName);
    }

private:

    const char* onGetName() final { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDelayedSetup() final {
        SkRandom rand;
        SkTArray<SkRect> rects;
        for (int i = 0; i < fRectCount; ++i) {
            rects.push_back(SkRect::MakeXYWH(rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 2000),
                                              rand.nextRangeF(0, 70),
                                              rand.nextRangeF(0, 70)));
        }

        fRects = fAlignedAllocator.makeArray<Rect>(fRectCount);
        for (int i = 0; i < fRectCount; ++i) {
            fRects[i] = rects[i];
        }
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void doBench() {
        SkRandom r;
        CompressedPaintersOrder order = CompressedPaintersOrder::First();
        CompressedPaintersOrder maxRead = CompressedPaintersOrder::First();
        for (int i = 0; i < fRectCount; ++i) {
            bool insert = r.nextBool();
            if (insert) {
                order = order.next();
                fManager->recordDraw(fRects[i], order, PaintersDepth::First(), false);
            } else {
                maxRead = std::max(maxRead,
                    fManager->getMostRecentDraw(fRects[i]));
            }
        }

        fMaxRead = maxRead;
        fManager->reset();
    }

    std::unique_ptr<BoundsManager> fManager;
    SkString fName;
    SkArenaAlloc fAlignedAllocator{0};
    int fRectCount;
    Rect* fRects;

    CompressedPaintersOrder fMaxRead;
};

}  // namespace skgpu::graphite

DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(100); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(500); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(1000); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(5000); )
DEF_BENCH( return new skgpu::graphite::RandomIntersectionBench(10000); )
DEF_BENCH( return new skgpu::graphite::FileIntersectionBench(); )  // Sniffs --intersectionTreeFile


DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 10000); )


DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 10000); )

// D = 17 leads to 4^16 leaf nodes, which overflows int (and is 4gb)... so probably need to cut off much soonr
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 100); )
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 1000); )
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 10000); )
