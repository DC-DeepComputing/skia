/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkPathTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ResourceProvider.h"

#ifdef SK_METAL
#include "src/gpu/graphite/mtl/MtlTrampoline.h"
#endif

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PaintCombo {
public:
    PaintCombo() {
        this->reset();
    }

private:
    friend class Context; // for iterators

    // SkBlenders
    void add(SkBlendMode bm) { fBlendModes.insert((uint32_t) bm); }
    void add(SkBlenderID id) { fBlendModes.insert(id.asUInt()); }

    // SkShaders
    void add(const ShaderCombo& shaderCombo) { fShaders.push_back(shaderCombo); }

    void reset() {
        fShaders.clear();
        fBlendModes.clear();
    }

    std::vector<ShaderCombo> fShaders;
    std::set<uint32_t> fBlendModes;
};

//--------------------------------------------------------------------------------------------------
void CombinationBuilder::add(ShaderCombo shaderCombo) {
    fCombination.add(shaderCombo);
}

void CombinationBuilder::add(SkBlendMode bm) { fCombination.add(bm); }
void CombinationBuilder::add(SkBlendMode rangeStart, SkBlendMode rangeEnd) {  // inclusive
    for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
        this->add((SkBlendMode) i);
    }
}
void CombinationBuilder::add(BlendModeGroups group) {
    switch (group) {
        case BlendModeGroups::kPorterDuff:
            this->add(SkBlendMode::kClear, SkBlendMode::kScreen);
            break;
        case BlendModeGroups::kAdvanced:
            this->add(SkBlendMode::kOverlay, SkBlendMode::kMultiply);
            break;
        case BlendModeGroups::kTrouble:
            this->add(SkBlendMode::kHue, SkBlendMode::kLuminosity);
            break;
        case BlendModeGroups::kAll:
            this->add(SkBlendMode::kClear, SkBlendMode::kLastMode);
            break;
    }
}

const PaintCombo* CombinationBuilder::snap() {
    if (!fCombination.fBlendModes) {
        fCombination.add(SkBlendMode::kSrcOver);
    }
    return &fCombination;
}
void CombinationBuilder::reset() {
    fCombination.reset();
}

//--------------------------------------------------------------------------------------------------
Context::Context(sk_sp<Gpu> gpu, BackendApi backend)
        : fGpu(std::move(gpu))
        , fGlobalCache(sk_make_sp<GlobalCache>())
        , fBackend(backend) {
}
Context::~Context() {}

#ifdef SK_METAL
std::unique_ptr<Context> Context::MakeMetal(const MtlBackendContext& backendContext) {
    sk_sp<Gpu> gpu = MtlTrampoline::MakeGpu(backendContext);
    if (!gpu) {
        return nullptr;
    }

    return std::unique_ptr<Context>(new Context(std::move(gpu), BackendApi::kMetal));
}
#endif

std::unique_ptr<Recorder> Context::makeRecorder() {
    return std::unique_ptr<Recorder>(new Recorder(fGpu, fGlobalCache));
}

void Context::insertRecording(const InsertRecordingInfo& info) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    SkASSERT(info.fRecording);
    if (!info.fRecording) {
        if (callback) {
            callback->setFailureResult();
        }
        return;
    }

    SkASSERT(!fCurrentCommandBuffer);
    // For now we only allow one CommandBuffer. So we just ref it off the InsertRecordingInfo and
    // hold onto it until we submit.
    fCurrentCommandBuffer = info.fRecording->fCommandBuffer;
    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }
}

void Context::submit(SyncToCpu syncToCpu) {
    SkASSERT(fCurrentCommandBuffer);

    fGpu->submit(std::move(fCurrentCommandBuffer));

    fGpu->checkForFinishedWork(syncToCpu);
}

void Context::checkAsyncWorkCompletion() {
    fGpu->checkForFinishedWork(SyncToCpu::kNo);
}

SkBlenderID Context::addUserDefinedSnippet(SkBlender* blender) {
    auto dict = this->priv().shaderCodeDictionary();

    return dict->addUserDefinedSnippet(blender);
}

void Context::preCompile(const PaintCombo& paintCombo) {
    static const Renderer* kRenderers[] = {
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kWinding),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kEvenOdd),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kInverseWinding),
            &Renderer::StencilTessellatedCurvesAndTris(SkPathFillType::kInverseEvenOdd),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kWinding),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kEvenOdd),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kInverseWinding),
            &Renderer::StencilTessellatedWedges(SkPathFillType::kInverseEvenOdd)
    };

    SkShaderCodeDictionary* dict = fGlobalCache->shaderCodeDictionary();
    SkKeyContext keyContext(dict);

    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    for (uint32_t bmVal : paintCombo.fBlendModes) {
        SkBlendMode bm;
        if (bmVal < kSkBlendModeCount) {
            bm = (SkBlendMode) bmVal;
        } else {
            continue;
        }

        for (const ShaderCombo& shaderCombo : paintCombo.fShaders) {
            for (auto shaderType: shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    // TODO: expand CreateKey to take either an SkBlendMode or an SkBlendID
                    auto uniqueID = CreateKey(keyContext, &builder, shaderType, tm, bm);

                    GraphicsPipelineDesc desc;

                    for (const Renderer* r : kRenderers) {
                        for (auto&& s : r->steps()) {
                            if (s->performsShading()) {
                                desc.setProgram(s, uniqueID);
                            }
                            // TODO: Combine with renderpass description set to generate full
                            // GraphicsPipeline and MSL program. Cache that compiled pipeline on
                            // the resource provider in a map from desc -> pipeline so that any
                            // later desc created from equivalent RenderStep + Combination get it.
                        }
                    }
                }
            }
        }
    }
    // TODO: Iterate over the renderers and make descriptions for the steps that don't perform
    // shading, and just use ShaderType::kNone.
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

} // namespace skgpu::graphite
