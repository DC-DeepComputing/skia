/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCombinationBuilder.h"

#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkMathPriv.h"
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
enum class UnusedShaderType { SHADER_TYPES(MAKE_ENUM) };
#undef MAKE_ENUM
static constexpr int kUnusedShaderTypeCount = static_cast<int>(UnusedShaderType::kBlendShader) + 1;

#define STATIC_ASSERT(T) static_assert((int) SkShaderType::k##T == (int) UnusedShaderType::k##T);
   SHADER_TYPES(STATIC_ASSERT)
#undef STATIC_ASSERT

static_assert(kSkShaderTypeCount == kUnusedShaderTypeCount);

//--------------------------------------------------------------------------------------------------
namespace {

int tm_pair_to_index(SkTileModePair tileModes) {
    static_assert(kSkTileModeCount <= 4);

    return (int) tileModes.fX << 2 | (int) tileModes.fY;
}

#ifdef SK_DEBUG
SkTileModePair index_to_tm_pair(int index) {
    static_assert(kSkTileModeCount <= 4);
    SkASSERT(index < 16);

    int tmX = index >> 2;
    int tmY = index & 3;

    return { (SkTileMode) tmX, (SkTileMode) tmY };
}

void add_indent(int indent) {
    SkDebugf("%*c", indent, ' ');
}

const char* type_to_str(SkShaderType type) {
    switch (type) {
#define CASE(T) case SkShaderType::k##T: return #T;
        SHADER_TYPES(CASE)
#undef CASE
    }

    SkUNREACHABLE;
}

#endif // SK_DEBUG

} // anonymous namespace

//--------------------------------------------------------------------------------------------------

// The base class for all the objects stored in the arena
// The base option object consists of some number of child slots each of which is an array of
// options. The slots and their associated linked list of options are allocated separately in
// the arena.
class SkOption {
public:
    // The SkSlot holds the combinatorial options for a single child slot of an
    // option (in a linked list).
    class SkSlot {
    public:
        void addOption(SkOption* newOption);

        int numCombinations() const;

        void createKey1(const SkKeyContext&, SkPaintParamsKeyBuilder*, int desiredCombination);

#ifdef SK_DEBUG
        void dump(int indent) const;
#endif

    private:
        SkOption* fHead = nullptr;
        SkOption* fTail = nullptr;
    };

    SkOption(SkShaderType type, int numSlots)
            : fType(type)
            , fNumSlots(numSlots) {}

    SkShaderType type() const { return fType; }
    int numSlots() const { return fNumSlots; }

#ifdef SK_DEBUG
    int epoch() const { return fEpoch; }
    void setEpoch(int epoch) {
        SkASSERT(fEpoch == kInvalidEpoch && epoch != kInvalidEpoch);
        fEpoch = epoch;
    }
#endif

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

    void addOption(int slotIndex, SkOption* newOption) {
        if (slotIndex >= fNumSlots) {
            return;
        }

        SkSlot* slot = this->getSlot(slotIndex);
        slot->addOption(newOption);
    }

    int numChildCombinations() const {
        int numChildCombinations = 1;
        for (int i = 0; i < fNumSlots; ++i) {
            numChildCombinations *= fSlots[i].numCombinations();
        }
        return numChildCombinations;
    }

    int numCombinations() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    void createKey2(const SkKeyContext& keyContext,
                    SkPaintParamsKeyBuilder* keyBuilder,
                    int desiredCombination) {
        SkASSERT(desiredCombination < this->numCombinations());

        int intrinsicCombination = desiredCombination / this->numChildCombinations();
        int childState = desiredCombination % this->numChildCombinations();

        this->addToKey(keyContext, keyBuilder, intrinsicCombination);

        if (fNumSlots) {
            int numCombosSeen = 0;
            for (int slotIndex = 0; slotIndex < fNumSlots; ++slotIndex) {
                SkSlot* slot = this->getSlot(slotIndex);

                numCombosSeen += slot->numCombinations();
                int numCombosLeft = this->numChildCombinations() / numCombosSeen;

                int slotState;
                if (slotIndex+1 < fNumSlots) {
                    slotState = childState / (numCombosLeft ? numCombosLeft : 1);
                    childState %= numCombosLeft;
                } else {
                    slotState = childState;
                }

                slot->createKey1(keyContext, keyBuilder, slotState);
            }
        }
    }

#ifdef SK_DEBUG
    void dump(int indent = 0) const {
        SkDebugf("%s", type_to_str(fType));
        if (!this->numIntrinsicCombinations()) {
            SkDebugf("(%d)", this->numIntrinsicCombinations());
        }
        SkDebugf(" { %d, %d }", this->numCombinations(), this->numChildCombinations());
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
    void addToKey(const SkKeyContext&, SkPaintParamsKeyBuilder*, int intrinsicCombination) const;

    SkDEBUGCODE(static constexpr int kInvalidEpoch = -1;)

    const SkShaderType fType;
    const int          fNumSlots;
    SkDEBUGCODE(int    fEpoch = kInvalidEpoch;)
    SkOption*          fNext = nullptr;
    SkSlot*            fSlots = nullptr;  // an array of 'fNumSlots' SkSlots
};

void SkOption::SkSlot::addOption(SkOption* newOption) {
    SkASSERT(newOption->fNext == nullptr);

    if (fHead == nullptr) {
        SkASSERT(fTail == nullptr);
        fHead = fTail = newOption;
    } else {
        SkASSERT(fTail->fNext == nullptr);
        fTail->fNext = newOption;
        fTail = newOption;
    }
}

int SkOption::SkSlot::numCombinations() const {
    int numCombinations = 0;

    for (SkOption* option = fHead; option; option = option->fNext) {
        numCombinations += option->numCombinations();
    }

    return numCombinations;
}

void SkOption::SkSlot::createKey1(const SkKeyContext& keyContext,
                                  SkPaintParamsKeyBuilder* keyBuilder,
                                  int desiredCombination) {
    SkASSERT(desiredCombination < this->numCombinations());

    for (SkOption* option = fHead; option; option = option->fNext) {
        if (desiredCombination < option->numCombinations()) {
            option->createKey2(keyContext, keyBuilder, desiredCombination);
            return;
        }

        desiredCombination -= option->numCombinations();
    }
}

#ifdef SK_DEBUG
void SkOption::SkSlot::dump(int indent) const {
    SkDebugf("{ %d } ", this->numCombinations());

    for (const SkOption* option = fHead; option; option = option->fNext) {
        option->dump(indent);
        SkDebugf(" | ");
    }
    SkDebugf("\n");
}
#endif

#define CREATE_ARENA_OBJECT(T, numChildSlots, ...)                                          \
struct ArenaData_##T : public SkOption {                                                    \
    static const SkShaderType kType = SkShaderType::k##T;                                   \
    static const int kNumChildSlots = numChildSlots;                                        \
    int numIntrinsicCombinationsDerived() const;                                            \
    void addToKey(const SkKeyContext&, SkPaintParamsKeyBuilder*, int intrinsicCombination); \
    __VA_ARGS__                                                                             \
};

CREATE_ARENA_OBJECT(SolidColor,      /* numChildSlots */ 0)
int ArenaData_SolidColor::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_SolidColor::addToKey(const SkKeyContext& keyContext,
                                    SkPaintParamsKeyBuilder* builder,
                                    int intrinsicCombination) {
    constexpr SkPMColor4f kUnusedColor = { 1, 0, 0, 1 };

    SkASSERT(intrinsicCombination == 0);
    SolidColorShaderBlock::AddToKey1(keyContext, builder, /*gatherer=*/nullptr, kUnusedColor);
    SolidColorShaderBlock::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(LinearGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_LinearGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_LinearGradient::addToKey(const SkKeyContext& keyContext,
                                        SkPaintParamsKeyBuilder* builder,
                                        int intrinsicCombination) {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                                   { SkShader::kLinear_GradientType,
                                     fMinNumStops + intrinsicCombination });
    GradientShaderBlocks::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(RadialGradient,  /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_RadialGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_RadialGradient::addToKey(const SkKeyContext& keyContext,
                                        SkPaintParamsKeyBuilder* builder,
                                        int intrinsicCombination) {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                                   { SkShader::kRadial_GradientType,
                                     fMinNumStops + intrinsicCombination });
    GradientShaderBlocks::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(SweepGradient,   /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_SweepGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_SweepGradient::addToKey(const SkKeyContext& keyContext,
                                       SkPaintParamsKeyBuilder* builder,
                                       int intrinsicCombination) {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                                   { SkShader::kSweep_GradientType,
                                     fMinNumStops + intrinsicCombination });
    GradientShaderBlocks::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(ConicalGradient, /* numChildSlots */ 0,
                    int fMinNumStops;
                    int fMaxNumStops;)
int ArenaData_ConicalGradient::numIntrinsicCombinationsDerived() const {
    return fMaxNumStops - fMinNumStops + 1;
}
void ArenaData_ConicalGradient::addToKey(const SkKeyContext& keyContext,
                                         SkPaintParamsKeyBuilder* builder,
                                         int intrinsicCombination) {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    GradientShaderBlocks::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                                   { SkShader::kConical_GradientType,
                                     fMinNumStops + intrinsicCombination });
    GradientShaderBlocks::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(LocalMatrix,     /* numChildSlots */ 1)
int ArenaData_LocalMatrix::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_LocalMatrix::addToKey(const SkKeyContext& keyContext,
                                     SkPaintParamsKeyBuilder* builder,
                                     int intrinsicCombination) {
    SkASSERT(intrinsicCombination == 0);

    LocalMatrixShaderBlock::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                                     // only the proxy shader is used!!!!
                                     { nullptr, SkMatrix::I() });
    LocalMatrixShaderBlock::AddToKey2(keyContext, builder);
}

// Split out due to constructor work
struct ArenaData_Image : public SkOption {
    static const SkShaderType kType = SkShaderType::kImage;
    static const int kNumChildSlots = 0;

    ArenaData_Image(const SkOption& init, SkSpan<SkTileModePair> tileModePairs)
            : SkOption(init)
            , fTileModeCombos(0) {
        for (auto tmPair : tileModePairs) {
            int index = tm_pair_to_index(tmPair);

#ifdef SK_DEBUG
            SkASSERT(index < 32);
            SkTileModePair tmp = index_to_tm_pair(index);
            SkASSERT(tmp == tmPair);
#endif

            fTileModeCombos |= 0x1 << index;
        }
    }

    int numIntrinsicCombinationsDerived() const;
    void addToKey(const SkKeyContext&, SkPaintParamsKeyBuilder*, int intrinsicCombination);

    int32_t fTileModeCombos;
};
int ArenaData_Image::numIntrinsicCombinationsDerived() const {
    return SkPopCount(fTileModeCombos);
}
void ArenaData_Image::addToKey(const SkKeyContext& keyContext,
                               SkPaintParamsKeyBuilder* builder,
                               int intrinsicCombination) {
    SkASSERT(intrinsicCombination < this->numIntrinsicCombinationsDerived());

    ImageShaderBlock::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                               // none of the ImageData is used
                               { SkSamplingOptions(),
                                 SkTileMode::kClamp, SkTileMode::kClamp,
                                 SkRect::MakeEmpty(), SkMatrix::I() });
    ImageShaderBlock::AddToKey2(keyContext, builder);
}

CREATE_ARENA_OBJECT(BlendShader,     /* numChildSlots */ 2)
int ArenaData_BlendShader::numIntrinsicCombinationsDerived() const { return 1; }
void ArenaData_BlendShader::addToKey(const SkKeyContext& keyContext,
                                     SkPaintParamsKeyBuilder* builder,
                                     int intrinsicCombination) {
    SkASSERT(intrinsicCombination == 0);

    BlendShaderBlock::AddToKey1(keyContext, builder, /*gatherer=*/nullptr,
                               // both the dst and src are used
                               { nullptr, nullptr, SkBlendMode::kSrc });
    BlendShaderBlock::AddToKey2(keyContext, builder);
}

// Here to access the derived ArenaData objects
int SkOption::numIntrinsicCombinations() const {
    int numIntrinsicCombinations;

    switch (this->type()) {
#define CASE(T) case SkShaderType::k##T:                                                   \
                    numIntrinsicCombinations =  ((ArenaData_##T*) this)->numIntrinsicCombinationsDerived(); \
                    break;
        SHADER_TYPES(CASE)
#undef CASE
    }

    SkASSERT(numIntrinsicCombinations >= 1); // There is always, at least, the existential combo
    return numIntrinsicCombinations;
}

// Here to access the derived ArenaData objects
void SkOption::addToKey(const SkKeyContext& keyContext,
                        SkPaintParamsKeyBuilder* builder,
                        int intrinsicCombination) const {
    switch (this->type()) {
#define CASE(T) case SkShaderType::k##T:                                     \
                    ((ArenaData_##T*) this)->addToKey(keyContext,            \
                                                      builder,               \
                                                      intrinsicCombination); \
                    break;
        SHADER_TYPES(CASE)
#undef CASE
    }
}

//--------------------------------------------------------------------------------------------------
SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    SkOption* child = fBuilder->addOptionInternal(type);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
}

SkCombinationOption SkCombinationOption::addChildOption(int childIndex, SkShaderType type,
                                                        int minNumStops, int maxNumStops) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    SkOption* child = fBuilder->addOptionInternal(type, minNumStops, maxNumStops);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
}

SkCombinationOption SkCombinationOption::addChildOption(
        int childIndex, SkShaderType type,
        SkSpan<SkTileModePair> tileModes) {
    if (!this->isValid() || childIndex >= this->numChildSlots()) {
        return SkCombinationOption(fBuilder, /*dataInArena=*/nullptr);
    }

    SkASSERT(fDataInArena->epoch() == fBuilder->fEpoch);

    SkOption* child = fBuilder->addOptionInternal(type, tileModes);
    if (child) {
        fDataInArena->addOption(childIndex, child);
    }

    return SkCombinationOption(fBuilder, child);
}

SkShaderType SkCombinationOption::type() const { return fDataInArena->type(); }
int SkCombinationOption::numChildSlots() const { return fDataInArena->numSlots(); }
SkDEBUGCODE(int SkCombinationOption::epoch() const { return fDataInArena->epoch(); })

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

    SkASSERT(arenaObject->type() == T::kType);
    SkASSERT(arenaObject->numSlots() == T::kNumChildSlots);

    SkDEBUGCODE(arenaObject->setEpoch(fEpoch));

    if (T::kNumChildSlots) {
        arenaObject->setSlotsArray(fArena->makeArrayDefault<SkOption::SkSlot>(T::kNumChildSlots));
    }

    return arenaObject;
}

void SkCombinationBuilder::addOption(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fBlendModes |= (0x1 << (int) bm);
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

    fBlenders.add(id);
}

SkOption* SkCombinationBuilder::addOptionInternal(SkShaderType shaderType) {

    // TODO: Can we use the X macro trick here to collapse this
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
}

SkOption* SkCombinationBuilder::addOptionInternal(SkShaderType shaderType,
                                                  int minNumStops, int maxNumStops) {

    // TODO: Can we use the X macro trick here to collapse this
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
}

SkOption* SkCombinationBuilder::addOptionInternal(SkShaderType shaderType,
                                                  SkSpan<SkTileModePair> tileModes) {

    // TODO: Can we use the X macro trick here to collapse this
    switch (shaderType) {
        case SkShaderType::kImage:
            return this->allocInArena<ArenaData_Image>(tileModes);
        default:
            return nullptr;
    }
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType) {

    SkOption* newOption = this->addOptionInternal(shaderType);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType,
                                                    int minNumStops, int maxNumStops) {

    SkOption* newOption = this->addOptionInternal(shaderType, minNumStops, maxNumStops);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

SkCombinationOption SkCombinationBuilder::addOption(SkShaderType shaderType,
                                                    SkSpan<SkTileModePair> tileModes) {

    SkOption* newOption = this->addOptionInternal(shaderType, tileModes);
    if (newOption) {
        fShaderOptions.push_back(newOption);
    }

    return { this, newOption };
}

void SkCombinationBuilder::reset() {
    fShaderOptions.reset();
    fBlendModes = 0;
    fBlenders.reset();
    fArena->reset();
    SkDEBUGCODE(++fEpoch;)
}

int SkCombinationBuilder::numBlendModeCombos() const {
    return SkPopCount(fBlendModes) + fBlenders.count();
}

int SkCombinationBuilder::numCombinations() const {
    int numShaderCombinations = 0;
    for (SkOption* s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    return numShaderCombinations * this->numBlendModeCombos();
}

#ifdef SK_DEBUG
void SkCombinationBuilder::dump() const {
    for (SkOption* s : fShaderOptions) {
        s->dump();
        SkDebugf("\n");
    }
}
#endif

// Here we strip off the unwanted bits and then return the number of trailing zero bits
static int get_index_of_ith_set_bit(uint32_t n, int i) {
    SkASSERT(i < SkPopCount(n));

    for (int j = 0; j < i; ++j) {
        n &= (n - 1); // Remove the lowest bit in the integer.
    }

    return SkCTZ(n);
}

#include "include/utils/SkRandom.h"

static void unit_test() {
    {
        uint32_t testVal = 0x1;
        int result = get_index_of_ith_set_bit(testVal, 0);
        SkASSERT(testVal == (0x1 << result));
    }

    {
        uint32_t testVal = 0x80000000;
        int result = get_index_of_ith_set_bit(testVal, 0);
        SkASSERT(testVal == (0x1 << result));
    }

    {
        uint32_t testVal = 0x55555555;
        uint32_t recreated = 0;
        for (int i = 0; i < 16; ++i) {
            int result = get_index_of_ith_set_bit(testVal, i);
            SkASSERT(result == 2*i);
            recreated |= (0x1 << result);
        }
        SkASSERT(testVal == recreated);
    }

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        int expectedNumSetBits = 0;
        uint32_t testVal = 0;

        int numTries = rand.nextULessThan(33);
        for (int j = 0; j < numTries; ++j) {
            int bit = rand.nextRangeU(0, 31);

            if (testVal & (0x1 << bit)) {
                continue;
            }

            ++expectedNumSetBits;
            testVal |= 0x1 << bit;
        }

        SkASSERT(SkPopCount(testVal) == expectedNumSetBits);
        uint32_t recreated = 0;

        for (int j = 0; j < expectedNumSetBits; ++j) {
            int index = get_index_of_ith_set_bit(testVal, j);
            recreated |= (0x1 << index);
        }

        SkASSERT(recreated == testVal);
    }
}

void SkCombinationBuilder::createKey(const SkKeyContext& keyContext,
                                     int state,
                                     SkPaintParamsKeyBuilder* keyBuilder) const {
#if 1
    unit_test();
#endif

    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(state < this->numCombinations());

    int numBlendModeCombos = this->numBlendModeCombos();

    int desiredShaderCombo = state / numBlendModeCombos;
    int blendCombo = state % numBlendModeCombos;

    for (SkOption* shaderOption : fShaderOptions) {
        if (desiredShaderCombo < shaderOption->numCombinations()) {
            shaderOption->createKey2(keyContext, keyBuilder, desiredShaderCombo);
            break;
        }

        desiredShaderCombo -= shaderOption->numCombinations();
    }

    if (blendCombo < SkPopCount(fBlendModes)) {
        int ith_set_bit = get_index_of_ith_set_bit(fBlendModes, blendCombo);

        SkASSERT(ith_set_bit < kSkBlendModeCount);
        SkBlendMode bm = (SkBlendMode) ith_set_bit;

        BlendModeBlock::AddToKey1(keyContext, keyBuilder, /*gatherer=*/nullptr, bm);
        BlendModeBlock::AddToKey2(keyContext, keyBuilder);
    } else {
        // TODO: need to handle fBlenders here
    }

}

void SkCombinationBuilder::buildCombinations(
        SkShaderCodeDictionary* dict,
        const std::function<void(SkUniquePaintParamsID)>& func) const {
    SkKeyContext keyContext(dict);
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    int numCombos = this->numCombinations();
    for (int i = 0; i < numCombos; ++i) {
        this->createKey(keyContext, i, &builder);

        SkPaintParamsKey key = builder.lockAsKey();

#ifdef SK_GRAPHITE_ENABLED
//        SkBlendMode bm = SkBlendMode::kSrcOver;
        SkPipelineDataGatherer::BlendInfo blendInfo; // = get_blend_info(bm);
#endif

        auto entry = dict->findOrCreate(
                key
#ifdef SK_GRAPHITE_ENABLED
                , blendInfo
#endif
        );

        func(entry->uniqueID());
    }

    for (int i = 0; i < kSkBlendModeCount; ++i) {
        if (!(fBlendModes & (0x1 << i))) {
            continue;
        }

        SkBlendMode bm = (SkBlendMode) i;

        // TODO: actually iterate over the SkOption's combinations and have each option add
        // itself to the key.
        for (SkOption* shaderOption : fShaderOptions) {
              // TODO: expand CreateKey to take either an SkBlendMode or an SkBlendID
              SkUniquePaintParamsID uniqueID = CreateKey1(keyContext, &builder,
                                                         shaderOption->type(), bm);

              func(uniqueID);
        }
    }

    // TODO: need to loop over fBlenders here
}
