/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Recording.h"
#include "experimental/graphite/src/Renderer.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

namespace skgpu {

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

sk_sp<Recorder> Context::createRecorder() {
    return sk_make_sp<Recorder>(sk_ref_sp(this));
}

void Context::insertRecording(std::unique_ptr<Recording> recording) {
    fRecordings.emplace_back(std::move(recording));
}

void Context::submit(SyncToCpu syncToCpu) {
    // TODO: we want Gpu::submit to take an array of command buffers but, for now, it just takes
    // one. Once we have more than one recording queued up we will need to extract the
    // command buffers and submit them as a block.
    SkASSERT(fRecordings.size() == 1);
    fGpu->submit(fRecordings[0]->fCommandBuffer);

    fGpu->checkForFinishedWork(syncToCpu);
    fRecordings.clear();
}

static SkBlendMode foo(BlendModeCombinations bm) {
    switch (bm) {
        case kSrcOver:
            return SkBlendMode::kSrcOver;
        case kSrc:
            return SkBlendMode::kSrc;
    }

    return SkBlendMode::kSrc;
}

const ShaderCombo2::Iter ShaderCombo2::Iter::kEnd = { true, {0, 0, 0}};

//std::vector<Combination>
void PaintCombinations::getCombos() const {
    for (auto& sc : fShaderCombos) {
        for (ShaderCombo2::Iter iter = sc->begin(); iter != ShaderCombo2::Iter::kEnd; ) {

            int code = sc->getCode(iter);
            for (int b = 0x01; b <= kMax; b <<= 1) {
                if (!(fBlendModes & b)) {
                    continue;
                }
                SkBlendMode bm = foo((BlendModeCombinations) b);

                SkDebugf("%d %d\n", code, (int) bm);
                //            Combination c {shaderType, tm, bm};

            }
        }
    }
}

void Context::preCompile(const PaintCombinations& combinations) {

//    std::vector<Combination> combos =
    combinations.getCombos();
//    for (Combination c : combos) {
//
    //}

//    for (auto bm: combinations.fBlendModes) {
        //for (auto& shaderCombo: combinations.fShaderCombos) {
#if 0
            for (auto shaderType: shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    Combination c {shaderType, tm, bm};

                    GraphicsPipelineDesc desc;

                    for (const Renderer* r : {&Renderer::StencilAndFillPath()}) {
                        for (auto&& s : r->steps()) {
                            desc.setProgram(s, c);
                            // TODO: Combine with renderpass description set to generate full
                            // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                            // the resource provider in a map from desc -> pipeline so that any
                            // later desc created from equivalent RenderStep + Combination get it.
                        }
                    }
                }
            }
#endif
        //}
//    }
}

} // namespace skgpu
