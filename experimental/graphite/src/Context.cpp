/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/Context.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/Recording.h"
#include "experimental/graphite/include/TextureInfo.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GlobalCache.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "include/core/SkPathTypes.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "src/core/SkKeyHelpers.h"

#ifdef SK_METAL
#include "experimental/graphite/src/mtl/MtlTrampoline.h"
#endif

namespace skgpu {

Context::Context(sk_sp<Gpu> gpu, BackendApi backend)
        : fGpu(std::move(gpu))
        , fGlobalCache(sk_make_sp<GlobalCache>())
        , fBackend(backend) {
}
Context::~Context() {}

#ifdef SK_METAL
std::unique_ptr<Context> Context::MakeMetal(const mtl::BackendContext& backendContext) {
    sk_sp<Gpu> gpu = mtl::Trampoline::MakeGpu(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return std::unique_ptr<Context>(new Context(std::move(gpu), BackendApi::kMetal));
}
#endif

std::unique_ptr<Recorder> Context::makeRecorder() {
    return std::unique_ptr<Recorder>(new Recorder(fGpu, fGlobalCache));
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

const ShaderCombo2::Iter ShaderCombo2::Iter::kEnd = { true, {0, 0, 0} };

PaintCombinations::Iter PaintCombinations::begin() const {
    if (fShaderCombos.empty()) {
        return { false, {0, FirstCombinableBlendMode(fCombinableBlendModes)}, nullptr, ShaderCombo2::Iter::kEnd };
    } else {
        return { false, {0, FirstCombinableBlendMode(fCombinableBlendModes)}, fShaderCombos[0].get(), fShaderCombos[0]->begin() };
    }
}

const PaintCombinations::Iter PaintCombinations::Iter::kEnd = { true, {0, 0} };

#ifdef SK_DEBUG
void PaintCombinations::checkIter(const Iter& iter) const {
    SkASSERT(!iter.fDone);
    SkASSERT(iter.fIndices[0] < (int) fShaderCombos.size());
    SkASSERT(iter.fIndices[1] > 0);
    SkASSERT(iter.fIndices[1] <= CombinableBlendMode::kLast2);
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

        for (; iter->fShaderIter != ShaderCombo2::Iter::kEnd; iter->fLastShaderCombo->next(&iter->fShaderIter)) {
            for (; iter->fIndices[1] <= CombinableBlendMode::kLast2; iter->fIndices[1] <<= 1) {
                if (!(fCombinableBlendModes & iter->fIndices[1])) {
                    continue;
                }

                if (first) {
                    first = false;
                    continue;
                }

                return;
            }

            iter->fIndices[1] = FirstCombinableBlendMode(fCombinableBlendModes);
        }
    }

    *iter = Iter::kEnd;
}

void PaintCombinations::getCode(SkShaderCodeDictionary* dict,
                                const Iter& iter,
                                SkBackend backend,
                                SkPaintParamsKey* key) const {
    SkDEBUGCODE(this->checkIter(iter));

//    for (auto& sc : fShaderCombos) {
//        for (ShaderCombo2::Iter iter = sc->begin(); iter != ShaderCombo2::Iter::kEnd; ) {

    iter.fLastShaderCombo->addToCode(dict, iter.fShaderIter, backend, key);

    SkBlendMode bm = CombinableBMtoBM((CombinableBlendMode) iter.fIndices[1]);
    BlendMode::addToKeyB(nullptr, backend, key, nullptr, bm);

//        }
//    }
}

void Context::preCompile(const PaintCombinations& combinations) {
    SkShaderCodeDictionary* dict = fGlobalCache->shaderCodeDictionary();

    static const Renderer* kRenderers[] = {
            &Renderer::StencilAndFillPath(SkPathFillType::kWinding),
            &Renderer::StencilAndFillPath(SkPathFillType::kEvenOdd),
            &Renderer::StencilAndFillPath(SkPathFillType::kInverseWinding),
            &Renderer::StencilAndFillPath(SkPathFillType::kInverseEvenOdd)
    };

    for (PaintCombinations::Iter iter = combinations.begin(); iter != combinations.end(); combinations.next(&iter)){
        SkPaintParamsKey key;

        combinations.getCode(dict, iter, SkBackend::kGraphite, &key);

        // key.dump();
        auto entry = dict->findOrCreate(key);

        for (const Renderer* r : kRenderers) {
            for (auto&& s : r->steps()) {
                if (s->performsShading()) {
                    GraphicsPipelineDesc desc;

                    desc.setProgram(s, entry->uniqueID());
                }

                // TODO: Combine with renderpass description set to generate full
                // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                // the resource provider in a map from desc -> pipeline so that any
                // later desc created from equivalent RenderStep + Combination get it.
            }
        }
    }
    // TODO: Iterate over the renderers and make descriptions for the steps that don't perform
    // shading, and just use ShaderType::kNone.
}

SkBlendMode CombinableBMtoBM(CombinableBlendMode combinableBM) {
    SkASSERT(combinableBM != CombinableBlendMode::kNone);

    switch (combinableBM) {
        case CombinableBlendMode::kClear:   return SkBlendMode::kClear;
        case CombinableBlendMode::kSrc:     return SkBlendMode::kSrc;
        case CombinableBlendMode::kDst:     return SkBlendMode::kDst;
        case CombinableBlendMode::kSrcOver: return SkBlendMode::kSrcOver;

        // maybe kNone?
        default:       return SkBlendMode::kSrc;
    }

    SkUNREACHABLE;
}

int FirstCombinableBlendMode(CombinableBlendMode combinableBlendModes) {
    int initialBM = 0x1;
    for ( ; initialBM <= CombinableBlendMode::kLast2; initialBM <<= 1) {
        if (combinableBlendModes & initialBM) {
            break;
        }
    }

    SkASSERT(initialBM <= CombinableBlendMode::kLast2);
    return initialBM;
}

BackendTexture Context::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    if (!info.isValid() || info.backend() != this->backend()) {
        return {};
    }
    return fGpu->createBackendTexture(dimensions, info);
}

void Context::deleteBackendTexture(BackendTexture& texture) {
    if (!texture.isValid() || texture.backend() != this->backend()) {
        return;
    }
    fGpu->deleteBackendTexture(texture);
}

} // namespace skgpu
