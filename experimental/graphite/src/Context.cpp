/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/Gpu.h"
#include "include/core/SkPaint.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

namespace skgpu {

namespace {

// TODO: Repurpose to assemble SkSL.
void expand(const ShaderCombo& c) {
    for (auto bm : c.fBlendModes) {
        for (auto& gc : c.fGradients) {
            for (auto g : gc.fTypes) {
                for (auto tm: gc.fTileModes) {
                    switch (g) {
                        case kNone_GradientType:
                            break;
                        case kColor_GradientType:
                            break;
                        case kLinear_GradientType:
                            break;
                        case kRadial_GradientType:
                            break;
                        case kSweep_GradientType:
                            break;
                        case kConical_GradientType:
                            break;

                    }
                    SkPaint p;
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
