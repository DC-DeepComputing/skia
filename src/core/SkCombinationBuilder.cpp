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

// To keep the SHADER_TYPES list and SkShaderType aligned, we create a hidden enum class
// and check it against SkShaderType
#define MAKE_ENUM(T) k##T,
enum class Unused_ShaderType { SHADER_TYPES(MAKE_ENUM) };
#undef MAKE_ENUM
static constexpr int kUnused_ShaderTypeCount = static_cast<int>(Unused_ShaderType::kBlendShader) + 1;

#define STATIC_ASSERT(T) static_assert((int) SkShaderType::k##T == (int) Unused_ShaderType::k##T);
   SHADER_TYPES(STATIC_ASSERT)
#undef STATIC_ASSERT

static_assert(kSkShaderTypeCount == kUnused_ShaderTypeCount);

//--------------------------------------------------------------------------------------------------
namespace {

int tm_pair_to_index(SkTileMode tm0, SkTileMode tm1) {
    return (int) tm0 * kSkTileModeCount + (int) tm1;
}

#ifdef SK_DEBUG
std::tuple<SkTileMode, SkTileMode> index_to_tm_pair(int index) {
    SkASSERT(index < 16);

    int tm0 = index / kSkTileModeCount;
    SkASSERT(tm0 < kSkTileModeCount);
    int tm1 = index % kSkTileModeCount;

    return { (SkTileMode) tm0, (SkTileMode) tm1 };
}

void add_indent(int indent) {
    for (int i = 0; i < indent; ++i) {
        SkDebugf(" ");
    }
}

const char* type_to_str(SkShaderType type) {
    switch (type) {
#define CASE(T) case SkShaderType::k##T: return #T;
        SHADER_TYPES(CASE)
#undef CASE
    }
}

#endif // SK_DEBUG

// Kernighan's method
int count_set_bits(uint32_t n) {
    int count = 0;

    while (n) {
        n &= (n - 1);
        count++;
    }
    return count;
}

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
// Objects stored in the arena

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

    int numCombinations() const;

#ifdef SK_DEBUG
    void dump(int indent) const;
#endif

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

    mutable int fNumCombinations = -1;
    int fNumOptions = 0;
    SkOptionsBlock fBlock;
};

// The base class for all the objects stored in the arena
// The base option object consists of some number of child slots each of which is an array of
// options. The slots and their associated list of blocks are allocated separately in the arena.
class SkOption {
public:
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

    int numChildCombinations() {
        if (fNumChildCombinations < 0) {
            fNumChildCombinations = 1;
            for (int i = 0; i < fNumSlots; ++i) {
                fNumChildCombinations *= fSlots[i].numCombinations();
            }
        }
        return fNumChildCombinations;
    }

    int numCombinations() {
        if (fNumCombinations < 0) {
            fNumCombinations = this->numIntrinsicCombinations() * this->numChildCombinations();
        }

        return fNumCombinations;
    }

#ifdef SK_DEBUG
    void dump(int indent = 0) const {
        SkDebugf("%s", type_to_str(fType));
        if (!this->numIntrinsicCombinations()) {
            SkDebugf("(%d)", this->numIntrinsicCombinations());
        }
        SkDebugf(" { %d, %d }", fNumCombinations, fNumChildCombinations);
        if (this->numSlots()) {
            SkDebugf("\n");
        }
        int childIndex = 0;
        for (int i = 0; i < fNumSlots; ++i) {
            add_indent(indent+4);
            SkDebugf("%d: ", childIndex);
            fSlots[i].dump(indent+4);
            ++childIndex;
        }
    }
#endif

private:
    int numIntrinsicCombinations() const;

    const SkShaderType fType;
    int                fNumCombinations = -1;
    int                fNumChildCombinations = -1;
    const int          fNumSlots;
    SkSlot*            fSlots = nullptr;  // an array of 'fNumSlots' SkSlots
};

// Here bc it needs to know about SkOption::numCombinations
int SkSlot::numCombinations() const {
    if (fNumCombinations < 0) {
        const SkOptionsBlock* block = &fBlock;
        fNumCombinations = 0;

        while (block) {
            for (int i = 0; i < kNumOptionsPerBlock; ++i) {
                if (block->fOptions[i]) {
                    fNumCombinations += block->fOptions[i]->numCombinations();
                }
            }
            block = block->fNext;
        }
    }

    return fNumCombinations;
}

#ifdef SK_DEBUG
// Here bc it needs to know about SkOption::dump
void SkSlot::dump(int indent) const {
    SkDebugf("{ %d } ", fNumCombinations);

    const SkOptionsBlock* block = &fBlock;
    while (block) {
        for (int i = 0; i < kNumOptionsPerBlock; ++i) {
            if (block->fOptions[i]) {
                block->fOptions[i]->dump(indent);
                SkDebugf(" | ");
            }
        }
        block = block->fNext;
    }
    SkDebugf("\n");
}
#endif

#define CREATE_ARENA_OBJECT(T, numChildSlots, ...)                         \
struct ArenaData_##T : public SkOption {                                   \
    static const SkShaderType kType = SkShaderType::k##T;                  \
    static const int kNumChildSlots = numChildSlots;                       \
    int numIntrinsicCombinations2() const;                                 \
    __VA_ARGS__                                                            \
};

CREATE_ARENA_OBJECT(SolidColor,      /* numChildSlots */ 0)
int ArenaData_SolidColor::numIntrinsicCombinations2() const { return 1; }

CREATE_ARENA_OBJECT(LinearGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_LinearGradient::numIntrinsicCombinations2() const {
    return fMaxNumStops - fMinNumStops + 1;
}

CREATE_ARENA_OBJECT(RadialGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_RadialGradient::numIntrinsicCombinations2() const {
    return fMaxNumStops - fMinNumStops + 1;
}

CREATE_ARENA_OBJECT(SweepGradient,   /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_SweepGradient::numIntrinsicCombinations2() const {
    return fMaxNumStops - fMinNumStops + 1;
}

CREATE_ARENA_OBJECT(ConicalGradient, /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_ConicalGradient::numIntrinsicCombinations2() const {
    return fMaxNumStops - fMinNumStops + 1;
}

CREATE_ARENA_OBJECT(LocalMatrix,     /* numChildSlots */ 1)
int ArenaData_LocalMatrix::numIntrinsicCombinations2() const { return 1; }

// Split out due to constructor work
struct ArenaData_Image : public SkOption {
    static const SkShaderType kType = SkShaderType::kImage;
    static const int kNumChildSlots = 0;

    ArenaData_Image(const SkOption& init,
                    const SkSpan<std::pair<SkTileMode, SkTileMode>>& tileModes)
            : SkOption(init)
            , fTileModeCombos(0) {
        for (auto p : tileModes) {
            int index = tm_pair_to_index(p.first, p.second);

#ifdef SK_DEBUG
            auto [t1, t2] = index_to_tm_pair(index);
            SkASSERT(t1 == p.first && t2 == p.second);
#endif

            fTileModeCombos |= 0x1 << index;
        }
    }

    int numIntrinsicCombinations2() const;

    int32_t fTileModeCombos;
};
int ArenaData_Image::numIntrinsicCombinations2() const { return count_set_bits(fTileModeCombos); }

CREATE_ARENA_OBJECT(BlendShader,     /* numChildSlots */ 2)
int ArenaData_BlendShader::numIntrinsicCombinations2() const { return 1; }

// Here to access the derived ArenaData objects
int SkOption::numIntrinsicCombinations() const {
    switch (this->type()) {
#define CASE(T) case SkShaderType::k##T: return ((ArenaData_##T*) this)->numIntrinsicCombinations2();
        SHADER_TYPES(CASE)
#undef ENUM
    }

    SkUNREACHABLE;
}

//--------------------------------------------------------------------------------------------------
// API-level wrapper around objects stored in the arena

SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, nullptr);
    }

    SkOption* child = fBuilder->addOptionInternal(type);
    if (child) {
        fDataInArena->addOption(fBuilder->arena(), childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
}

SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type,
                                                        int minNumStops, int maxNumStops) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, nullptr);
    }

    SkOption* child = fBuilder->addOptionInternal(type, minNumStops, maxNumStops);
    if (child) {
        fDataInArena->addOption(fBuilder->arena(), childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
}

SkCombinationOption SkCombinationOption::addChildOption(
        int childIndex, SkShaderType type,
        const SkSpan<std::pair<SkTileMode, SkTileMode>>& tileModes) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, nullptr);
    }

    SkOption* child = fBuilder->addOptionInternal(type, tileModes);
    if (child) {
        fDataInArena->addOption(fBuilder->arena(), childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
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

template<typename T, typename... Args>
SkOption* SkCombinationBuilder::allocInArena(Args&&... args) {
    SkOption* arenaObject = fArena->make<T>(T{{ T::kType, T::kNumChildSlots },
                                              std::forward<Args>(args)... });
    if (!arenaObject) {
        return nullptr;
    }

    if (T::kNumChildSlots) {
        arenaObject->setSlotsArray(fArena->makeArrayDefault<SkSlot>(T::kNumChildSlots));
    }

    //fShaders.push_back(arenaObject);
    return arenaObject;
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

SkOption* SkCombinationBuilder::addOptionInternal(SkShaderType shaderType) {

    // TODO: Can we use the macro-of-macro trick here to collapse this
    switch (shaderType) {
        case SkShaderType::kSolidColor:
            return this->allocInArena<ArenaData_SolidColor>();
        case SkShaderType::kLocalMatrix:
            return this->allocInArena<ArenaData_LocalMatrix>();
        case SkShaderType::kBlendShader:
            return this->allocInArena<ArenaData_BlendShader>();
        default:
            return nullptr;
    }

    SkUNREACHABLE;
}

SkOption* SkCombinationBuilder::addOptionInternal(SkShaderType shaderType,
                                                  int minNumStops, int maxNumStops) {

    // TODO: Can we use the macro-of-macro trick here to collapse this
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
            return nullptr;
    }

    SkUNREACHABLE;
}

SkOption* SkCombinationBuilder::addOptionInternal(
        SkShaderType shaderType,
        const SkSpan<std::pair<SkTileMode, SkTileMode>>& tileModes) {

    // TODO: Can we use the macro-of-macro trick here to collapse this
    switch (shaderType) {
        case SkShaderType::kImage:
            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return nullptr;
    }

    SkUNREACHABLE;
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType) {

    SkOption* newOption = this->addOptionInternal(shaderType);
    if (newOption) {
        fShaders.push_back(newOption);
    }

    return { this, newOption };
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType,
                                                    int minNumStops, int maxNumStops) {

    SkOption* newOption = this->addOptionInternal(shaderType, minNumStops, maxNumStops);
    if (newOption) {
        fShaders.push_back(newOption);
    }

    return { this, newOption };
}

SkCombinationOption SkCombinationBuilder::addOption(
        SkShaderType shaderType,
        const SkSpan<std::pair<SkTileMode, SkTileMode>>& tileModes) {

    SkOption* newOption = this->addOptionInternal(shaderType, tileModes);
    if (newOption) {
        fShaders.push_back(newOption);
    }

    return { this, newOption };
}

void SkCombinationBuilder::reset() {
    fShaders.reset();
    fBlendModes.reset();
    fArena->reset();
}

int SkCombinationBuilder::numCombinations() const {
    int numShaderCombinations = 0;
    for (auto s : fShaders) {
        numShaderCombinations += s->numCombinations();
    }

    return numShaderCombinations * fBlendModes.count();
}

#ifdef SK_DEBUG
void SkCombinationBuilder::dump() const {
    for (auto s : fShaders) {
        s->dump();
        SkDebugf("\n");
    }
}
#endif

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
