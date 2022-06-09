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

    void onPerCanvasPostDraw(SkCanvas*) override {
// #if SK_DEBUG
        // SkDebugf("%s >> grouped %d draws into %d sets <<\n",
        //          fName.c_str(), fRectCount, fMaxRead.bits());
// #endif
    }

    void doBench() {
        PaintersDepth depth = PaintersDepth::First();
        CompressedPaintersOrder maxRead = CompressedPaintersOrder::First();
        for (int i = 0; i < fRectCount; ++i) {
            const Rect& drawBounds = fRects[i];
            CompressedPaintersOrder order = fManager->getMostRecentDraw(drawBounds).next();
            depth = depth.next();
            fManager->recordDraw(drawBounds, order, depth, false);
            if (order > maxRead) {
                maxRead = order;
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


DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::NaiveBoundsManager>(), "naive", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(std::make_unique<skgpu::graphite::BruteForceBoundsManager>(), "bruteForce", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 5), "quad5", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 7), "quad7", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 8), "quad8", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 10), "quad10", 10000); )

// D = 17 leads to 4^16 leaf nodes, which overflows int (and is 4gb)... so probably need to cut off much soonr
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 100); )
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 1000); )
// DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::QuadTreeBoundsManager::MakeDepth({1800, 1800}, 17), "quad17", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 16), "grid16", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 64), "grid64", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 128), "grid128", 10000); )

DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 10); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 100); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 250); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 1000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 2500); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 5000); )
DEF_BENCH( return new skgpu::graphite::BoundsManagerBench(skgpu::graphite::GridBoundsManager::Make({1800, 1800}, 512), "grid512", 10000); )

