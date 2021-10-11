/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/Gpu.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

// TODO: these are only included to play with shader combinations
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

namespace skgpu {

namespace {

// TODO: Repurpose to assemble SkSL?
void expand(const ShaderCombo& c) {
    SkPoint pts[2] = { { -100, -100 }, { 100, 100 } };
    SkColor colors[2] = { SK_ColorRED, SK_ColorGREEN };

    for (auto bm : c.fBlendModes) {
        for (auto& gradCombo : c.fGradients) {
            for (auto gradType : gradCombo.fTypes) {
                for (auto tm: gradCombo.fTileModes) {
                    sk_sp<SkShader> g;
                    switch (gradType) {
                        case GradientCombo::GradientType::kNone:
                            break;
                        case GradientCombo::GradientType::kLinear:
                            g = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, tm);
                            break;
                        case GradientCombo::GradientType::kRadial:
                            g = SkGradientShader::MakeRadial({0, 0}, 100, colors, nullptr, 2, tm);
                            break;
                        case GradientCombo::GradientType::kSweep:
                            g = SkGradientShader::MakeSweep(0, 0, colors, nullptr, 2);
                            break;
                        case GradientCombo::GradientType::kConical:
                            g = SkGradientShader::MakeTwoPointConical({100, 100}, 100,
                                                                      {-100, -100}, 100,
                                                                      colors, nullptr, 2, tm);
                            break;
                    }
                    SkPaint p;
                    p.setShader(std::move(g));
                    p.setBlendMode(bm);
                }
            }
        }
    }
}

} // anonymous namespace

Context::Context(sk_sp<Gpu> gpu) : fGpu(std::move(gpu)) {}
Context::~Context() {}

#ifdef SK_METAL
sk_sp<Context> Context::MakeMetal(const mtl::BackendContext& backendContext) {
    sk_sp<Gpu> gpu = mtl::Trampoline::MakeGpu(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return sk_sp<Context>(new Context(std::move(gpu)));
}
#endif

void Context::preCompile(const ShaderCombo& c) {
    expand(c);
}

} // namespace skgpu
