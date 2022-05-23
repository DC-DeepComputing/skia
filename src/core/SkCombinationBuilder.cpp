/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCombinationBuilder.h"

#include "include/private/SkTHash.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/ContextPriv.h"
#endif

//--------------------------------------------------------------------------------------------------
class SkPaintCombinations {
public:
    SkPaintCombinations() {}

    // SkBlenders
    void add(SkBlendMode bm) { fBlendModes.add((uint32_t) bm); }
    void add(SkBlenderID id) { fBlendModes.add(id.asUInt()); }
    int numBlendModes() const { return fBlendModes.count(); }

    // SkShaders
    void add(const SkShaderCombo& shaderCombo) { fShaders.push_back(shaderCombo); }

private:
    friend class SkCombinationBuilder;     // for iterators

    std::vector<SkShaderCombo> fShaders;
    SkTHashSet<uint32_t>       fBlendModes;
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
    static const SkShaderType kType = SkShaderType::k##T;                  \
    static const int kNumChildSlots = 0;                                   \
    __VA_ARGS__                                                            \
};

#define CREATE_ARENA_OBJECT_WITH_KIDS(T, numChildSlots, ...)               \
struct ArenaData_##T : public ArenaData_Base {                             \
    static const SkShaderType kType = SkShaderType::k##T;                  \
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

API_LevelClass API_LevelClass::addChildOption(int childIndex, SkShaderType type) {
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

API_LevelClass API_LevelClass::addChildOption(int childIndex, SkShaderType type,
                                              int minNumStops, int maxNumStops) {
    return API_LevelClass(fBuilder, nullptr);
}

API_LevelClass API_LevelClass::addChildOption(int childIndex, SkShaderType type,
                                              const SkSpan<SkTileMode>& tileModes) {
    return API_LevelClass(fBuilder, nullptr);
}

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkCombinationBuilder::SkCombinationBuilder(skgpu::graphite::Context* context)
        : fDictionary(context->priv().shaderCodeDictionary()) {
    fArena = std::make_unique<SkArenaAllocWithReset>(64);
    this->reset();
}
#else
SkCombinationBuilder::SkCombinationBuilder(SkShaderCodeDictionary* dict)
        : fDictionary(dict) {
    fArena = std::make_unique<SkArenaAllocWithReset>(64);
    this->reset();
}
#endif

#if 0
void SkCombinationBuilder::addOption(SkShaderCombo shaderCombo) {
    fCombinations->add(shaderCombo);
}
#endif

template<typename T, typename... Args>
API_LevelClass  SkCombinationBuilder::allocInArena(Args&&... args) {
    // In Arena!!
    ArenaData_Base* arenaObject = new T{ { T::kType, T::kNumChildSlots }, std::forward<Args>(args)...};
    if (!arenaObject) {
        return API_LevelClass(this, nullptr);
    }

    fShaders.push_back(arenaObject);
    return API_LevelClass(this, arenaObject);
}

void SkCombinationBuilder::addOption(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fCombinations->add(bm);
}

void SkCombinationBuilder::addOption(SkBlendMode rangeStart, SkBlendMode rangeEnd) {
    for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
        this->addOption((SkBlendMode) i);
    }
}

void SkCombinationBuilder::addOption(BlendModeGroup group) {
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

void SkCombinationBuilder::addOption(SkBlenderID id) {
    SkASSERT(fDictionary->isValidID(id.asUInt()));

    fCombinations->add(id);
}

API_LevelClass SkCombinationBuilder::addOption(SkShaderType shaderType) {
    switch (shaderType) {
        case SkShaderType::kSolidColor:
            return this->allocInArena<ArenaData_SolidColor>();
        case SkShaderType::kLocalMatrix:
            return this->allocInArena<ArenaData_LocalMatrix>();
        case SkShaderType::kBlendShader:
            return this->allocInArena<ArenaData_BlendShader>();
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}

API_LevelClass SkCombinationBuilder::addOption(SkShaderType shaderType,
                                               int minNumStops, int maxNumStops) {
    switch (shaderType) {
        case SkShaderType::kLinearGradient:
            return this->allocInArena<ArenaData_LinearGradient>(minNumStops, maxNumStops);
        case SkShaderType::kRadialGradient:
            return this->allocInArena<ArenaData_RadialGradient>(minNumStops, maxNumStops);
        case SkShaderType::kSweepGradient:
            return this->allocInArena<ArenaData_SweepGradient>(minNumStops, maxNumStops);
        case SkShaderType::kConicalGradient:
            return this->allocInArena<ArenaData_ConicalGradient>(minNumStops, maxNumStops);
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}

API_LevelClass SkCombinationBuilder::addOption(SkShaderType shaderType,
                                               const SkSpan<SkTileMode>& tileModes) {
    switch (shaderType) {
//        case SkShaderType::kImage:
//            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return API_LevelClass(this, nullptr);
    }

    SkUNREACHABLE;
}

void SkCombinationBuilder::reset() {
    fArena->reset();
    fCombinations = fArena->make<SkPaintCombinations>();
}

void SkCombinationBuilder::buildCombinations(
        SkShaderCodeDictionary* dict,
        const std::function<void(SkUniquePaintParamsID)>& func) const {
    SkKeyContext keyContext(dict);
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    for (uint32_t bmVal : fCombinations->fBlendModes) {
        SkBlendMode bm;
        if (bmVal < kSkBlendModeCount) {
            bm = (SkBlendMode) bmVal;
        } else {
            // TODO: add creation of PaintParamKey fragments from runtime effect SkBlenders
            continue;
        }

        for (const SkShaderCombo& shaderCombo : fCombinations->fShaders) {
            for (auto shaderType: shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    // TODO: expand CreateKey to take either an SkBlendMode or an SkBlendID
                    auto uniqueID = CreateKey(keyContext, &builder, shaderType, tm, bm);

                    func(uniqueID);
                }
            }
        }
    }
}
