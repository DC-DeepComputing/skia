/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Context.h"

#include "include/core/SkPathTypes.h"
#include "include/effects/SkRuntimeEffect.h"
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

#include <cstdlib>
#include <stdarg.h>

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PaintCombinations {
public:
    PaintCombinations() {}

    // SkBlenders
    void add(SkBlendMode bm) { fBlendModes.add((uint32_t) bm); }
    void add(SkBlenderID id) { fBlendModes.add(id.asUInt()); }
    int numBlendModes() const { return fBlendModes.count(); }

    // SkShaders
    void add(const ShaderCombo& shaderCombo) { fShaders.push_back(shaderCombo); }

private:
    friend class Context; // for iterators

    std::vector<ShaderCombo> fShaders;
    SkTHashSet<uint32_t> fBlendModes;
};

//--------------------------------------------------------------------------------------------------
// Objects stored in the arena


void ArenaData_Base::addOption(int childIndex, ArenaData_Base* newOption) {
    if (childIndex >= fNumChildSlots) {
        return;
    }

}

#define CREATE_ARENA_OBJECT_NOKIDS(T, ...)                                 \
struct ArenaData_##T : public ArenaData_Base {                             \
    static const ShaderType kType = ShaderType::k##T;                      \
    static const int kNumChildSlots = 0;                                   \
    __VA_ARGS__                                                            \
};

#define CREATE_ARENA_OBJECT_WITH_KIDS(T, numChildSlots, ...)               \
struct ArenaData_##T : public ArenaData_Base {                             \
    static const ShaderType kType = ShaderType::k##T;                      \
    static const int kNumChildSlots = numChildSlots;                       \
    __VA_ARGS__                                                            \
    int fCurNumOptions[numChildSlots] = {};                                \
    ArenaData_Base* fChildSlots[numChildSlots][4] = {};                    \
};

CREATE_ARENA_OBJECT_NOKIDS(SolidColor)
CREATE_ARENA_OBJECT_NOKIDS(LinearGradient,
                           int fMinNumStops;
                           int fMaxNumStops;)
CREATE_ARENA_OBJECT_NOKIDS(RadialGradient,
                           int fMinNumStops;
                           int fMaxNumStops;)
CREATE_ARENA_OBJECT_NOKIDS(SweepGradient,
                           int fMinNumStops;
                           int fMaxNumStops;)
CREATE_ARENA_OBJECT_NOKIDS(ConicalGradient,
                           int fMinNumStops;
                           int fMaxNumStops;)
CREATE_ARENA_OBJECT_WITH_KIDS(LocalMatrix,     /* numChildSlots */ 1)
CREATE_ARENA_OBJECT_NOKIDS(Image,
                           int32_t fTileModeCombos;)
CREATE_ARENA_OBJECT_WITH_KIDS(BlendShader,     /* numChildSlots */ 2)

//--------------------------------------------------------------------------------------------------
// API-level wrapper around objects stored in the arena

API_LevelClass API_LevelClass::addChildOption(int childIndex, ShaderType type) {
    if (childIndex >= this->numChildSlots()) {
        return API_LevelClass(fBuilder, nullptr);
    }

    API_LevelClass child = fBuilder->addOption(type);
    if (!child.isValid()) {
        return API_LevelClass(fBuilder, nullptr);
    }

    fDataInArena->addOption(childIndex, child.fDataInArena);

    return child;
}

API_LevelClass API_LevelClass::addChildOption(int childIndex, ShaderType type,
                                              int minNumStops, int maxNumStops) {
    return API_LevelClass(fBuilder, nullptr);
}

API_LevelClass API_LevelClass::addChildOption(int childIndex, ShaderType type,
                                              const SkSpan<SkTileMode>& tileModes) {
    return API_LevelClass(fBuilder, nullptr);
}

//--------------------------------------------------------------------------------------------------
CombinationBuilder::CombinationBuilder(Context* context)
        : fDictionary(context->priv().shaderCodeDictionary()) {
    this->reset();
}

template<typename T, typename... Args>
API_LevelClass  CombinationBuilder::allocInArena(Args&&... args) {
    // In Arena!!
    ArenaData_Base* arenaObject = new T{ { T::kType, T::kNumChildSlots }, std::forward<Args>(args)...};
    if (!arenaObject) {
        return API_LevelClass(this, nullptr);
    }

    fShaders.push_back(arenaObject);
    return API_LevelClass(this, arenaObject);
}

void CombinationBuilder::addOption(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fCombinations->add(bm);
}

void CombinationBuilder::addOption(SkBlendMode rangeStart, SkBlendMode rangeEnd) {
    for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
        this->addOption((SkBlendMode) i);
    }
}

void CombinationBuilder::addOption(BlendModeGroup group) {
    switch (group) {
        case BlendModeGroup::kPorterDuff:
            this->addOption(SkBlendMode::kClear, SkBlendMode::kScreen);
            break;
        case BlendModeGroup::kAdvanced:
            this->addOption(SkBlendMode::kOverlay, SkBlendMode::kMultiply);
            break;
        case BlendModeGroup::kColorAware:
            this->addOption(SkBlendMode::kHue, SkBlendMode::kLuminosity);
            break;
        case BlendModeGroup::kAll:
            this->addOption(SkBlendMode::kClear, SkBlendMode::kLastMode);
            break;
    }
}

void CombinationBuilder::addOption(SkBlenderID id) {
    SkASSERT(fDictionary->isValidID(id.asUInt()));

    fCombinations->add(id);
}

API_LevelClass CombinationBuilder::addOption(ShaderType shaderType) {
    switch (shaderType) {
        case ShaderType::kSolidColor:
            return this->allocInArena<ArenaData_SolidColor>();
        case ShaderType::kLocalMatrix:
            return this->allocInArena<ArenaData_LocalMatrix>();
        case ShaderType::kBlendShader:
            return this->allocInArena<ArenaData_BlendShader>();
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}

API_LevelClass CombinationBuilder::addOption(ShaderType shaderType,
                                             int minNumStops, int maxNumStops) {
    switch (shaderType) {
        case ShaderType::kLinearGradient:
            return this->allocInArena<ArenaData_LinearGradient>(minNumStops, maxNumStops);
        case ShaderType::kRadialGradient:
            return this->allocInArena<ArenaData_RadialGradient>(minNumStops, maxNumStops);
        case ShaderType::kSweepGradient:
            return this->allocInArena<ArenaData_SweepGradient>(minNumStops, maxNumStops);
        case ShaderType::kConicalGradient:
            return this->allocInArena<ArenaData_ConicalGradient>(minNumStops, maxNumStops);
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}

API_LevelClass CombinationBuilder::addOption(ShaderType shaderType,
                                             const SkSpan<SkTileMode>& tileModes) {
    switch (shaderType) {
//        case ShaderType::kImage:
//            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}


void CombinationBuilder::reset() {
    fArena.reset();
    fCombinations = fArena.make<PaintCombinations>();
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

SkBlenderID Context::addUserDefinedBlender(sk_sp<SkRuntimeEffect> effect) {
    auto dict = this->priv().shaderCodeDictionary();

    return dict->addUserDefinedBlender(std::move(effect));
}

void Context::preCompile(const CombinationBuilder& combinationBuilder) {
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

    for (uint32_t bmVal : combinationBuilder.fCombinations->fBlendModes) {
        SkBlendMode bm;
        if (bmVal < kSkBlendModeCount) {
            bm = (SkBlendMode) bmVal;
        } else {
            // TODO: add creation of PaintParamKey fragments from runtime effect SkBlenders
            continue;
        }

        for (const ShaderCombo& shaderCombo : combinationBuilder.fCombinations->fShaders) {
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
