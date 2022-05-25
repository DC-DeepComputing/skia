/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCombinationBuilder.h"

#include <tuple>
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/ContextPriv.h"
#endif

// TODO: move out of header
#define SHADER_TYPES(M)             \
    M(SolidColor)                   \
                                    \
    M(LinearGradient)               \
    M(RadialGradient)               \
    M(SweepGradient)                \
    M(ConicalGradient)              \
                                    \
    M(LocalMatrix)                  \
    M(Image)                        \
    M(BlendShader)

#if 0
#define ENUM(T) k##T,
enum class ShaderType { SHADER_TYPES(ENUM) };
#undef ENUM
#endif

//--------------------------------------------------------------------------------------------------
// Objects stored in the arena

// The base class for all the objects stored in the arena
// The base option object consists of some number of child slots each of which is an array of
// options. The child arrays are implemented as a blocky linked list.
class SkOption {
public:
    class SkSlot {
    public:
        int numOptions() const { return fNumOptions; }

        void addOption(SkArenaAlloc* arena, SkOption* newOption) {
            int remainingRoom = kNumOptionsPerBlock - (fNumOptions % kNumOptionsPerBlock);
            if (!remainingRoom) {
                // allocate a new block
                this->addNewBlock(arena);
            }

            auto [block, index] = this->findEmptyOptionField();
            SkASSERT(index < kNumOptionsPerBlock);
            SkASSERT(!block->fOptions[index]);

            block->fOptions[index] = newOption;
            ++fNumOptions;
        }

    private:
        static const int kNumOptionsPerBlock = 4;

        struct SkOptionsBlock {
            SkOption* fOptions[kNumOptionsPerBlock] = {};
            SkOptionsBlock* fNext = nullptr;
        };

        void addNewBlock(SkArenaAlloc* arena) {
            SkOptionsBlock* block = &fBlock;
            while (block->fNext) {
                block = block->fNext;
            }

            block->fNext = arena->make<SkOptionsBlock>();
        }

        std::tuple<SkOptionsBlock*, int> findEmptyOptionField() {
            SkOptionsBlock* block = &fBlock;
            int index = fNumOptions;
            while(index >= kNumOptionsPerBlock) {
                block = block->fNext;
                index -= kNumOptionsPerBlock;
            }

            return std::tuple(block, index);
        }

        int fNumOptions = 0;
        SkOptionsBlock fBlock;
    };

    SkOption(SkShaderType type, int numSlots)
            : fType(type)
            , fNumSlots(numSlots) {}

    SkShaderType type() const { return fType; }
    int numSlots() const { return fNumSlots; }

    void setSlotsArray(SkSlot* slots) {
        SkASSERT(!fSlots);
        fSlots = slots;
    }

    SkSlot* getSlot(int slotIndex) {
        if (slotIndex >= fNumSlots) {
            return nullptr;
        }

        return &fSlots[slotIndex];
    }

    void addOption(SkArenaAlloc* arena, int slotIndex, SkOption* newOption) {
        if (slotIndex >= fNumSlots) {
            return;
        }

        SkSlot* slot = this->getSlot(slotIndex);
        slot->addOption(arena, newOption);
    }

private:
    const SkShaderType fType;
    const int          fNumSlots;
    SkSlot*            fSlots = nullptr;  // an array of 'fNumSlots' SkSlots
};

#define CREATE_ARENA_OBJECT(T, numChildSlots, ...)                         \
struct ArenaData_##T : public SkOption {                                   \
    static const SkShaderType kType = SkShaderType::k##T;                  \
    static const int kNumChildSlots = numChildSlots;                       \
    __VA_ARGS__;                                                           \
                                                                           \
    template <typename Functor>                                            \
    void visit(Functor&& f) const {                                        \
        return f(this);                                                    \
    }                                                                      \
};

CREATE_ARENA_OBJECT(SolidColor,      /* numChildSlots */ 0)
CREATE_ARENA_OBJECT(LinearGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
CREATE_ARENA_OBJECT(RadialGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
CREATE_ARENA_OBJECT(SweepGradient,   /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
CREATE_ARENA_OBJECT(ConicalGradient, /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
CREATE_ARENA_OBJECT(LocalMatrix,     /* numChildSlots */ 1)
CREATE_ARENA_OBJECT(Image,           /* numChildSlots */ 0,
                    int32_t fTileModeCombos;)
CREATE_ARENA_OBJECT(BlendShader,     /* numChildSlots */ 2)

//--------------------------------------------------------------------------------------------------
// API-level wrapper around objects stored in the arena

SkCombinationOption SkCombinationOption::addChildOption(int childIndex,
                                                        SkShaderType type) {
    if (childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, nullptr);
    }

    SkCombinationOption child = fBuilder->addOption(type);
    if (!child.isValid()) {
        return SkCombinationOption(fBuilder, nullptr);
    }

    fDataInArena->addOption(fBuilder->arena(), childIndex, child.fDataInArena);

    return child;
}

SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type,
                                                        int minNumStops, int maxNumStops) {
    return SkCombinationOption(fBuilder, nullptr);
}

SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type,
                                                        const SkSpan<SkTileMode>& tileModes) {
    return SkCombinationOption(fBuilder, nullptr);
}

SkShaderType SkCombinationOption::type() const { return fDataInArena->type(); }
int SkCombinationOption::numChildSlots() const { return fDataInArena->numSlots(); }

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
SkCombinationOption SkCombinationBuilder::allocInArena(Args&&... args) {
    SkOption* arenaObject = fArena->make<T>( { T::kType, T::kNumChildSlots }, std::forward<Args>(args)...);
    if (!arenaObject) {
        return SkCombinationOption(this, nullptr);
    }

    if (arenaObject->numSlots()) {
        arenaObject->setSlotsArray(fArena->makeArrayDefault<SkOption::SkSlot>(arenaObject->numSlots()));
    }

    fShaders.push_back(arenaObject);
    return SkCombinationOption(this, arenaObject);
}

void SkCombinationBuilder::addOption(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fBlendModes.add((uint32_t) bm);
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

    fBlendModes.add(id.asUInt());
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType) {
    switch (shaderType) {
        case SkShaderType::kSolidColor:
            return this->allocInArena<ArenaData_SolidColor>();
        case SkShaderType::kLocalMatrix:
            return this->allocInArena<ArenaData_LocalMatrix>();
        case SkShaderType::kBlendShader:
            return this->allocInArena<ArenaData_BlendShader>();
        default:
            return SkCombinationOption(this, nullptr);
    }

    SkUNREACHABLE;
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType,
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
            return SkCombinationOption(this, nullptr);
    }

    SkUNREACHABLE;
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType,
                                                    const SkSpan<SkTileMode>& tileModes) {
    switch (shaderType) {
//        case SkShaderType::kImage:
//            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return SkCombinationOption(this, nullptr);
    }

    SkUNREACHABLE;
}

void SkCombinationBuilder::reset() {
    fArena->reset();
}

void SkCombinationBuilder::buildCombinations(
        SkShaderCodeDictionary* dict,
        const std::function<void(SkUniquePaintParamsID)>& func) const {
    SkKeyContext keyContext(dict);
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    for (uint32_t bmVal : fBlendModes) {
        SkBlendMode bm;
        if (bmVal < kSkBlendModeCount) {
            bm = (SkBlendMode) bmVal;
        } else {
            // TODO: add creation of PaintParamKey fragments from runtime effect SkBlenders
            continue;
        }

        for (SkOption* shaderOption : fShaders) {
              SkTileMode tm = SkTileMode::kClamp;
//            for (auto tm: shaderCombo.fTileModes) {
                // TODO: expand CreateKey to take either an SkBlendMode or an SkBlendID
                auto uniqueID = CreateKey(keyContext, &builder, shaderOption->type(), tm, bm);

                func(uniqueID);
//            }
        }
    }
}
