/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCombinationBuilder_DEFINED
#define SkCombinationBuilder_DEFINED

#include <functional>
#include <memory>
#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"

class SkArenaAllocWithReset;
class SkPaintCombinations;
class SkPaintParamsKeyBuilder;
class SkShaderCodeDictionary;
class SkUniquePaintParamsID;

namespace skgpu::graphite {
class Context;
}

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

enum class SkShaderType : uint32_t {
    kSolidColor,

    kLinearGradient,
    kRadialGradient,
    kSweepGradient,
    kConicalGradient,

    kLocalMatrix,
    kImage,
    kBlendShader
};

#if 0
// TODO: remove this class
struct SkShaderCombo {
    SkShaderCombo() {}
    SkShaderCombo(std::vector<SkShaderType> types,
                  std::vector<SkTileMode> tileModes)
            : fTypes(std::move(types))
            , fTileModes(std::move(tileModes)) {
    }
    std::vector<SkShaderType> fTypes;
    std::vector<SkTileMode> fTileModes;
};
#endif

// TODO: add SkShaderID and SkColorFilterID too
class SkBlenderID {
public:
    SkBlenderID() : fID(0) {}  // 0 is an invalid blender ID
    SkBlenderID(const SkBlenderID& src) : fID(src.fID) {}

    bool isValid() const { return fID > 0; }

    bool operator==(const SkBlenderID& other) const { return fID == other.fID; }

    SkBlenderID& operator=(const SkBlenderID& src) {
        fID = src.fID;
        return *this;
    }

private:
    friend class SkShaderCodeDictionary;   // for ctor and asUInt access
    friend class SkCombinationBuilder;       // for asUInt access
    friend class SkPaintCombinations;        // for asUInt access

    SkBlenderID(uint32_t id) : fID(id) {}

    uint32_t asUInt() const { return fID; }

    uint32_t fID;
};

class ArenaData_Base {
public:
    ArenaData_Base(SkShaderType type, int numChildSlots)
            : fType(type)
            , fNumChildSlots(numChildSlots) {}

    void addOption(int childIndex, ArenaData_Base* newOption);

    const SkShaderType fType;
    const int          fNumChildSlots;
};

class CombinationBuilder;

class API_LevelClass {
public:
    API_LevelClass(CombinationBuilder* builder, ArenaData_Base* dataInArena)
            : fBuilder(builder)
            , fDataInArena(dataInArena) {}

    API_LevelClass addChildOption(int childIndex, SkShaderType);

    API_LevelClass addChildOption(int childIndex, SkShaderType,
                                  int minNumStops, int maxNumStops);

    API_LevelClass addChildOption(int childIndex, SkShaderType,
                                  const SkSpan<SkTileMode>&);

    bool isValid() const { return fDataInArena; }
    SkShaderType type() const { return fDataInArena->fType; }
    int numChildSlots() const { return fDataInArena->fNumChildSlots; }

private:
    CombinationBuilder* fBuilder;
    ArenaData_Base* fDataInArena;
};

class SkCombinationBuilder {
public:
    enum class BlendModeGroup {
        kPorterDuff,         // [ kClear .. kScreen ]
        kAdvanced,           // [ kOverlay .. kMultiply ]
        kColorAware,         // [ kHue .. kLuminosity ]
        kAll
    };

#ifdef SK_GRAPHITE_ENABLED
    SkCombinationBuilder(skgpu::graphite::Context*);
#else
    SkCombinationBuilder(SkShaderCodeDictionary*);
#endif

    // Blend Modes
    void addOption(SkBlendMode);
    void addOption(SkBlendMode rangeStart, SkBlendMode rangeEnd); // inclusive
    void addOption(BlendModeGroup);
    void addOption(SkBlenderID);

    // Shaders
    API_LevelClass addOption(SkShaderType shaderType);

    API_LevelClass addOption(SkShaderType shaderType, int minNumStops, int maxNumStops);

    API_LevelClass addOption(SkShaderType shaderType, const SkSpan<SkTileMode>& tileModes);

    void reset();

private:
    friend class skgpu::graphite::Context; // for access to buildCombinations

    template<typename T, typename... Args>
    API_LevelClass allocInArena(Args&&... args);

    void buildCombinations(SkShaderCodeDictionary*,
                           const std::function<void(SkUniquePaintParamsID)>&) const;

    SkShaderCodeDictionary* fDictionary;
    std::unique_ptr<SkArenaAllocWithReset> fArena;
    SkPaintCombinations* fCombinations;
    SkTArray<ArenaData_Base*> fShaders;
};

#endif // SkCombinationBuilder_DEFINED
