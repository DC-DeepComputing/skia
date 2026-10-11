/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <vector>
#include "bench/Benchmark.h"
#include "src/gpu/tessellate/ArcLengthCalculator.h"

namespace skgpu::graphite {
using ArcLengthCalculator = skgpu::tess::ArcLengthCalculator;
namespace {

struct ConicData {
    skvx::float2 p0, p1, p2;
    float w;
};

struct CubicData {
    skvx::float2 c0, c1, c2, c3;
};

struct LineData {
    skvx::float2 p0, p1;
};

enum class VerbType { kLine, kConic, kCubic };

struct VerbData {
    VerbType type;
    skvx::float2 pts[4];
    float w;
};

skvx::float2 make_sin_pt(float f, float dx, float freq = 1.0f) {
    return {f * 0.01f + dx, sinf(f * freq) * 10.f};
}

skvx::float2 make_cos_pt(float f, float dx, float freq = 1.0f) {
    return {f * 0.01f + dx, cosf(f * freq) * 10.f};
}

float make_conic_w(int i) { return 0.1f + 1.9f * (float)(i % 100) / 100.f; }

LineData make_line(int i) {
    float f = (float)i;
    return {make_sin_pt(f, 0.f), make_cos_pt(f, 5.f)};
}

ConicData make_conic(int i) {
    float f = (float)i;
    return {make_sin_pt(f, 0.f), make_cos_pt(f, 5.f), make_sin_pt(f, 10.f, 2.f), make_conic_w(i)};
}

CubicData make_cubic(int i) {
    float f = (float)i;
    return {make_sin_pt(f, 0.f),
            make_cos_pt(f, 3.f),
            make_sin_pt(f, 6.f, 2.f),
            make_cos_pt(f, 9.f, 2.f)};
}

class ConicArcLengthBench : public Benchmark {
public:
    ConicArcLengthBench() : fName("ArcLength_Conic") { this->setupConics(); }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        float sum = 0.0f;
        for (int i = 0; i < loops; ++i) {
            for (const auto& conic : fConics) {
                sum += ArcLengthCalculator::ConicLength(conic.p0, conic.p1, conic.p2, conic.w);
            }
        }
        fSum = sum;
    }

private:
    void setupConics() {
        fConics.reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            fConics.push_back(make_conic(i));
        }
    }

    SkString fName;
    std::vector<ConicData> fConics;
    volatile float fSum = 0.0f;
};

class CubicArcLengthBench : public Benchmark {
public:
    CubicArcLengthBench() : fName("ArcLength_Cubic") { this->setupCubics(); }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        float sum = 0.0f;
        for (int i = 0; i < loops; ++i) {
            for (const auto& cubic : fCubics) {
                sum += ArcLengthCalculator::CubicLength(cubic.c0, cubic.c1, cubic.c2, cubic.c3);
            }
        }
        fSum = sum;
    }

private:
    void setupCubics() {
        fCubics.reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            fCubics.push_back(make_cubic(i));
        }
    }

    SkString fName;
    std::vector<CubicData> fCubics;
    volatile float fSum = 0.0f;
};

class CombinedArcLengthBench : public Benchmark {
public:
    CombinedArcLengthBench() : fName("ArcLength_Combined") { this->setupVerbs(); }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override {
        float sum = 0.0f;
        for (int i = 0; i < loops; ++i) {
            for (const auto& verb : fVerbs) {
                switch (verb.type) {
                    case VerbType::kLine:
                        sum += ArcLengthCalculator::LineLength(verb.pts[0], verb.pts[1]);
                        break;
                    case VerbType::kConic:
                        sum += ArcLengthCalculator::ConicLength(
                                verb.pts[0], verb.pts[1], verb.pts[2], verb.w);
                        break;
                    case VerbType::kCubic:
                        sum += ArcLengthCalculator::CubicLength(
                                verb.pts[0], verb.pts[1], verb.pts[2], verb.pts[3]);
                        break;
                }
            }
        }
        fSum = sum;
    }

private:
    void setupVerbs() {
        fVerbs.reserve(15000);
        for (int i = 0; i < 15000; ++i) {
            VerbType type = static_cast<VerbType>(i % 3);
            if (type == VerbType::kLine) {
                auto line = make_line(i);
                fVerbs.push_back({
                        VerbType::kLine,
                        {line.p0, line.p1},
                        0.0f,
                });
            } else if (type == VerbType::kConic) {
                auto conic = make_conic(i);
                fVerbs.push_back({
                        VerbType::kConic,
                        {conic.p0, conic.p1, conic.p2},
                        conic.w,
                });
            } else {
                auto cubic = make_cubic(i);
                fVerbs.push_back({
                        VerbType::kCubic,
                        {cubic.c0, cubic.c1, cubic.c2, cubic.c3},
                        0.0f,
                });
            }
        }
    }

    SkString fName;
    std::vector<VerbData> fVerbs;
    volatile float fSum = 0.0f;
};

}  // namespace
}  // namespace skgpu::graphite

DEF_BENCH(return new skgpu::graphite::ConicArcLengthBench();)
DEF_BENCH(return new skgpu::graphite::CubicArcLengthBench();)
DEF_BENCH(return new skgpu::graphite::CombinedArcLengthBench();)
