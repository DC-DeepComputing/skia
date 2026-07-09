/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/private/SkTArray.h"
#include "src/core/SkRandom.h"
#include "src/gpu/ganesh/geometry/GrPathUtils.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"

#include <vector>

namespace {
using namespace skgpu::graphite;

enum class BenchmarkDataset {
    kTiger,
    kMixed,
};

constexpr float kMixedWidthF = 1000.f;
constexpr float kMixedHeightF = 1000.f;

static std::vector<SkPath> GetMixedPaths() {
    std::vector<SkPath> paths;
    SkRandom rand(42);
    constexpr int kNumPaths = 100;
    constexpr int kVerbsPerPath = 50;

    for (int i = 0; i < kNumPaths; ++i) {
        SkPathBuilder builder;
        builder.moveTo(rand.nextRangeF(0.f, kMixedWidthF), rand.nextRangeF(0.f, kMixedHeightF));
        for (int j = 0; j < kVerbsPerPath; ++j) {
            int type = rand.nextULessThan(4);
            switch (type) {
                case 0:
                    builder.lineTo(rand.nextRangeF(0.f, kMixedWidthF),
                                   rand.nextRangeF(0.f, kMixedHeightF));
                    break;
                case 1:
                    builder.quadTo(rand.nextRangeF(0.f, kMixedWidthF),
                                   rand.nextRangeF(0.f, kMixedHeightF),
                                   rand.nextRangeF(0.f, kMixedWidthF),
                                   rand.nextRangeF(0.f, kMixedHeightF));
                    break;
                case 2:
                    builder.conicTo(rand.nextRangeF(0.f, kMixedWidthF),
                                    rand.nextRangeF(0.f, kMixedHeightF),
                                    rand.nextRangeF(0.f, kMixedWidthF),
                                    rand.nextRangeF(0.f, kMixedHeightF),
                                    rand.nextRangeF(0.1f, 5.f));
                    break;
                case 3:
                    builder.cubicTo(rand.nextRangeF(0.f, kMixedWidthF),
                                    rand.nextRangeF(0.f, kMixedHeightF),
                                    rand.nextRangeF(0.f, kMixedWidthF),
                                    rand.nextRangeF(0.f, kMixedHeightF),
                                    rand.nextRangeF(0.f, kMixedWidthF),
                                    rand.nextRangeF(0.f, kMixedHeightF));
                    break;
            }
        }
        paths.push_back(builder.detach());
    }
    return paths;
}

using namespace skgpu::graphite;

template <FlattenMode kMode, BenchmarkDataset kDataset> class FlattenBench : public Benchmark {
public:
    FlattenBench() {
        if constexpr (kDataset == BenchmarkDataset::kTiger) {
            fPaths = Tiger::GetTigerPaths();
            fWidth = Tiger::kTigerWidthF;
            fHeight = Tiger::kTigerHeightF;
            fName.printf("SparseStrips_Flatten_%s_Tiger",
                         kMode == FlattenMode::kScalar ? "Scalar" : "SIMD");
        } else {
            fPaths = GetMixedPaths();
            fWidth = kMixedWidthF;
            fHeight = kMixedHeightF;
            fName.printf("SparseStrips_Flatten_%s_Mixed",
                         kMode == FlattenMode::kScalar ? "Scalar" : "SIMD");
        }
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        Flatten flattener;
        Polyline polyline;
        for (int i = 0; i < loops; ++i) {
            for (auto& path : fPaths) {
                flattener.processPaths<kMode>(path, SkMatrix(), fWidth, fHeight, &polyline);
            }
            polyline.reset();
        }
    }

private:
    SkString fName;
    std::vector<SkPath> fPaths;
    float fWidth;
    float fHeight;
};

template <BenchmarkDataset kDataset> class GrLegacyBench : public Benchmark {
public:
    // Match the tolerances of Flatten.
    static constexpr SkScalar kTolerance = Flatten::kQuadErrTolerance;
    static constexpr SkScalar kToleranceSqd = Flatten::kQuadTolerance2;

    GrLegacyBench() {
        if constexpr (kDataset == BenchmarkDataset::kTiger) {
            fPaths = Tiger::GetTigerPaths();
            fName.printf("SparseStrips_FlattenLegacy_Ganesh_Tiger");
        } else {
            fPaths = GetMixedPaths();
            fName.printf("SparseStrips_FlattenLegacy_Ganesh_Mixed");
        }
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        SkAutoConicToQuads converter;
        skia_private::TArray<SkPoint> pointBuffer;
        skia_private::TArray<SkPoint, true> cubicQuadsBuffer;

        for (int i = 0; i < loops; ++i) {
            for (auto& path : fPaths) {
                SkPath::Iter iter(path, false);
                SkPoint pts[4];
                SkPath::Verb verb;

                while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                    switch (verb) {
                        case SkPath::kMove_Verb:
                        case SkPath::kLine_Verb:
                            pointBuffer.push_back(pts[0]);
                            if (verb == SkPath::kLine_Verb) {
                                pointBuffer.push_back(pts[1]);
                            }
                            break;

                        case SkPath::kQuad_Verb: {
                            this->flattenQuad(pts, &pointBuffer);
                            break;
                        }

                        case SkPath::kConic_Verb: {
                            const SkPoint* quadPts =
                                    converter.computeQuads(pts, iter.conicWeight(), kTolerance);
                            int quadCount = converter.countQuads();
                            for (int j = 0; j < quadCount; ++j) {
                                this->flattenQuad(&quadPts[j * 2], &pointBuffer);
                            }
                            break;
                        }

                        case SkPath::kCubic_Verb: {
                            cubicQuadsBuffer.clear();
                            GrPathUtils::convertCubicToQuads(pts, kTolerance, &cubicQuadsBuffer);
                            int quadCount = cubicQuadsBuffer.empty()
                                                    ? 0
                                                    : (cubicQuadsBuffer.size() - 1) / 2;
                            for (int j = 0; j < quadCount; ++j) {
                                this->flattenQuad(&cubicQuadsBuffer[j * 2], &pointBuffer);
                            }
                            break;
                        }
                        case SkPath::kClose_Verb:
                        default:
                            break;
                    }
                }
            }
            pointBuffer.clear();
        }
    }

private:
    void flattenQuad(const SkPoint pts[3], skia_private::TArray<SkPoint>* pointBuffer) const {
        uint32_t count = GrPathUtils::quadraticPointCount(pts, kTolerance);
        SkPoint* outPtr = pointBuffer->push_back_n(count);
        GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2], kToleranceSqd, &outPtr, count);
    }

    SkString fName;
    std::vector<SkPath> fPaths;
};

}  // namespace

DEF_BENCH(return (new FlattenBench<skgpu::graphite::FlattenMode::kScalar,
                                   BenchmarkDataset::kTiger>());)
DEF_BENCH(
        return (new FlattenBench<skgpu::graphite::FlattenMode::kSimd, BenchmarkDataset::kTiger>());)
DEF_BENCH(return (new GrLegacyBench<BenchmarkDataset::kTiger>());)

DEF_BENCH(return (new FlattenBench<skgpu::graphite::FlattenMode::kScalar,
                                   BenchmarkDataset::kMixed>());)
DEF_BENCH(
        return (new FlattenBench<skgpu::graphite::FlattenMode::kSimd, BenchmarkDataset::kMixed>());)
DEF_BENCH(return (new GrLegacyBench<BenchmarkDataset::kMixed>());)
