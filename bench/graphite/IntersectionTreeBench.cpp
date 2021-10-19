/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "experimental/graphite/src/geom/IntersectionTree.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMathPriv.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include <vector>

static DEFINE_string(intersectionTreeFile, "",
                     "svg or skp for the IntersectionTree bench to sniff paths from.");

namespace skgpu {

class IntersectionTreeBench : public Benchmark {
protected:
    const char* onGetName() final { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }

    void onDraw(int loops, SkCanvas*) final {
        for (int i = 0; i < loops; ++i) {
            this->doBench();
        }
    }

    void doBench() {
        SkAutoTArray<Rect> storeA(fRects.size()), storeB(fRects.size());
        Rect* rects = fRects.data();
        Rect* collided = storeA.data();
        int rectCount = (int)fRects.size();
        volatile int numLoops = 0;
        fNumTrees = 0;
        while (rectCount > 0) {
            ++numLoops;
            IntersectionTree intersectionTree;
            int collidedCount = 0;
            for (int i = 0; i < rectCount; ++i) {
                if (!intersectionTree.add(rects[i])) {
                    collided[collidedCount++] = rects[i];
                }
            }
            std::swap(rects, collided);
            if (collided == fRects.data()) {
                collided = storeB.data();
            }
            rectCount = collidedCount;
            ++fNumTrees;
        }
    }

    SkString fName;
    std::vector<Rect> fRects;
    int fNumTrees = 0;
};

class RandomIntersectionBench : public IntersectionTreeBench {
public:
    RandomIntersectionBench(int numRandomRects) : fNumRandomRects(numRandomRects) {
        fName.printf("IntersectionTree_%i", numRandomRects);
    }

private:
    void onDelayedSetup() override {
        SkRandom rand;
        for (int i = 0; i < fNumRandomRects; ++i) {
            fRects.push_back(Rect::XYWH(rand.nextRangeF(0, 2000),
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

    void onDelayedSetup() override {
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
            Rect drawBounds = matrix.mapRect(path.getBounds());
            float gpuFragmentWork = drawBounds.area();
            float cpuTessellationWork = numVerbs * SkNextLog2(numVerbs);  // N log N.
            constexpr static float kCpuWeight = 512;
            constexpr static float kMinNumPixelsToTriangulate = 256 * 256;
            if (cpuTessellationWork * kCpuWeight + kMinNumPixelsToTriangulate < gpuFragmentWork) {
                return;  // Goes to inner triangulator.
            }
            fRects.push_back(drawBounds);
        });
        SkDebugf(">> Found %llu stencil/cover paths in %s <<\n",
                 fRects.size(), FLAGS_intersectionTreeFile[0]);
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        if (FLAGS_intersectionTreeFile.isEmpty()) {
            return;
        }
        SkDebugf(">> Reordered %s into %i different stencil/cover draws <<\n",
                 FLAGS_intersectionTreeFile[0], fNumTrees);
    }
};

}  // namespace skgpu

DEF_BENCH( return new skgpu::RandomIntersectionBench(100); )
DEF_BENCH( return new skgpu::RandomIntersectionBench(500); )
DEF_BENCH( return new skgpu::RandomIntersectionBench(1000); )
DEF_BENCH( return new skgpu::RandomIntersectionBench(5000); )
DEF_BENCH( return new skgpu::RandomIntersectionBench(10000); )
DEF_BENCH( return new skgpu::FileIntersectionBench(); )  // Sniffs --intersectionTreeFile
