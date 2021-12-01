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
    SkASSERT(bm != BlendModeCombinations::kNone);

    switch (bm) {
        case kClear:   return SkBlendMode::kClear;
        case kSrc:     return SkBlendMode::kSrc;
        case kDst:     return SkBlendMode::kDst;
        case kSrcOver: return SkBlendMode::kSrcOver;

        // maybe kNone?
        default:       return SkBlendMode::kSrc;
    }

    SkUNREACHABLE;
}

const ShaderCombo2::Iter ShaderCombo2::Iter::kEnd = { true, {0, 0, 0} };

PaintCombinations::Iter PaintCombinations::begin() const {
    int initialBM = 0x1;
    for ( ; initialBM <= kLast_BlendModes; initialBM <<= 1) {
        if (fBlendModes & initialBM) {
            break;
        }
    }

    SkASSERT(initialBM <= kLast_BlendModes);

    if (fShaderCombos.empty()) {
        return { false, {0, initialBM}, nullptr, ShaderCombo2::Iter::kEnd };
    } else {
        return { false, {0, initialBM}, fShaderCombos[0].get(), fShaderCombos[0]->begin() };
    }
}

const PaintCombinations::Iter PaintCombinations::Iter::kEnd = { true, {0, 0} };

#ifdef SK_DEBUG
void PaintCombinations::checkIter(const Iter& iter) const {
    SkASSERT(!iter.fDone);
    SkASSERT(iter.fIndices[0] < (int) fShaderCombos.size());
    SkASSERT(iter.fIndices[1] > 0);
    SkASSERT(iter.fIndices[1] <= kLast_BlendModes);
}
#endif

void PaintCombinations::next(Iter* iter) const {
    SkDEBUGCODE(this->checkIter(*iter));

    bool first = true;
    for (; iter->fIndices[0] < (int) fShaderCombos.size(); ++iter->fIndices[0]) {
        if (iter->fLastShaderCombo != fShaderCombos[iter->fIndices[0]].get()) {
            iter->fLastShaderCombo = fShaderCombos[iter->fIndices[0]].get();
            iter->fShaderIter = iter->fLastShaderCombo->begin();
        }

        for (; iter->fIndices[1] <= kLast_BlendModes; iter->fIndices[1] <<= 1) {
            if (!(fBlendModes & iter->fIndices[1])) {
                continue;
            }

            if (first) {
                first = false;
                continue;
            }

            return;
        }

        iter->fIndices[1] = 0x1;
    }

    *iter = Iter::kEnd;
}

void PaintCombinations::getCode(const Iter& iter, Combo2* combo) const {
    SkDEBUGCODE(this->checkIter(iter));

//    for (auto& sc : fShaderCombos) {
//        for (ShaderCombo2::Iter iter = sc->begin(); iter != ShaderCombo2::Iter::kEnd; ) {

    iter.fLastShaderCombo->addToCode(iter.fShaderIter, combo);

    {
        SkBlendMode bm = foo((BlendModeCombinations)iter.fIndices[1]);
        combo->addInt((int)bm);
    }
//        }
//    }
}

void Context::preCompile(const PaintCombinations& combinations) {

    for (PaintCombinations::Iter iter = combinations.begin(); iter != combinations.end(); combinations.next(&iter)){
        Combo2 combo;

        combinations.getCode(iter, &combo);

        combo.dump();
    }

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
}

static const char* to_str(SkBlendMode bm) {
    switch (bm) {
        case SkBlendMode::kClear: return "clear";
        case SkBlendMode::kSrc: return "src";
        case SkBlendMode::kDst: return "dst";
        case SkBlendMode::kSrcOver: return "srcOver";
        default: return "unknownBM";
    }

    SkUNREACHABLE;
}

void Combo2::dump() const {
    SkASSERT(this->numInts() == 2);

    SkDebugf("Combo: ");
    int iFoo = fInts[0] & kShaderCodeMask;
    switch (iFoo) {
        case ShaderCode::kNone_ShaderCode:
            NoShaderCombo::Dump(*this);
            break;
        case ShaderCode::kImage_ShaderCode:
            ImageShaderCombinations::Dump(*this);
            break;
        case ShaderCode::kGradient_ShaderCode:
            GradientCombinations::Dump(*this);
            break;
    }

    SkBlendMode bm = (SkBlendMode) fInts[1];
    SkDebugf(" %s\n", to_str(bm));
}

} // namespace skgpu
