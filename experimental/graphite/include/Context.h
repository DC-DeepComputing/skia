/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Context_DEFINED
#define skgpu_Context_DEFINED

#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

#include "experimental/graphite/include/GraphiteTypes.h"

namespace skgpu {

class ContextPriv;
class Gpu;
class Recorder;
class Recording;
namespace mtl { struct BackendContext; }




class Combo2 {
public:

private:
};

class ShaderCombo2 {
public:
#if 0
    class Iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = GrSubRun;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(GrSubRun* subRun) : fPtr{subRun} { }
        Iterator& operator++() { fPtr = fPtr->fNext.get(); return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fPtr == rhs.fPtr; }
        bool operator!=(const Iterator& rhs) const { return fPtr != rhs.fPtr; }
        reference operator*() { return *fPtr; }

    private:
        GrSubRun* fPtr;
    };

    Iterator begin() { return Iterator{ fHead.get()}; }
    Iterator end() { return Iterator{nullptr}; }
    Iterator begin() const { return Iterator{ fHead.get()}; }
    Iterator end() const { return Iterator{nullptr}; }
#endif

private:
};

class NoShaderCombo : public ShaderCombo2 {
public:
private:
};

class GradientCombinations : public ShaderCombo2 {
public:
    enum Types {
        kLinear = 0x01,
        kRadial = 0x02,
        kConical = 0x04,
        kSweep = 0x08,

        kAllTypes = kLinear | kRadial | kConical | kSweep
    };

    enum TileModes {
        kClamp = 0x01,
        kRepeat = 0x02,
        kMirror = 0x04,
        kDecal = 0x08,

        kAllTileModes = kClamp | kRepeat | kMirror | kDecal
    };

    GradientCombinations(Types types = kAllTypes)
            : fTypes(types)
            , fTileModes(TileModes::kAllTileModes)
            , fMinNumStops(2)
            , fMaxNumStops(256) {}
    GradientCombinations(Types types, unsigned int minNumStops, unsigned int maxNumStops = 256)
            : fTypes(types)
            , fMinNumStops(minNumStops)
            , fMaxNumStops(maxNumStops) {}
    GradientCombinations(unsigned int minNumStops, unsigned int maxNumStops = 256)
            : fTypes(Types::kAllTypes)
            , fTileModes(TileModes::kAllTileModes)
            , fMinNumStops(minNumStops)
            , fMaxNumStops(maxNumStops) {}

    void foo() const {


    }

private:
    Types fTypes;
    TileModes fTileModes;
    unsigned int fMinNumStops;
    unsigned int fMaxNumStops;
};

class ImageShaderType : public ShaderCombo2 {
public:

private:
};

#if 1
struct ShaderCombo {
    enum class ShaderType {
        kNone, // does not modify color buffer, e.g. depth and/or stencil only
        kSolidColor,
        kLinearGradient,
        kRadialGradient,
        kSweepGradient,
        kConicalGradient
    };

    ShaderCombo() {}
    ShaderCombo(std::vector<ShaderType> types,
                std::vector<SkTileMode> tileModes)
            : fTypes(std::move(types))
            , fTileModes(std::move(tileModes)) {
    }
    std::vector<ShaderType> fTypes;
    std::vector<SkTileMode> fTileModes;
};
#endif

enum BlendModeCombinations {
    kSrcOver = 0x1,
    kSrc = 0x2,
};

class PaintCombinations {
    void addShaderCombinations(std::unique_ptr<ShaderCombo2> combo) { fShaderCombos.emplace_back(std::move(combo)); }
    void addBlendModeCombinations(BlendModeCombinations blendModeCombinations) {
        fBlendModes = (BlendModeCombinations) (fBlendModes | blendModeCombinations);
    }

private:
    std::vector<std::unique_ptr<ShaderCombo2>> fShaderCombos;
//    std::vector<ImageFilterCombo> fIFCombos;
    BlendModeCombinations fBlendModes;
};

class Context final : public SkRefCnt {
public:
    ~Context() override;

#ifdef SK_METAL
    static sk_sp<Context> MakeMetal(const skgpu::mtl::BackendContext&);
#endif

    sk_sp<Recorder> createRecorder();

    void insertRecording(std::unique_ptr<Recording>);
    void submit(SyncToCpu = SyncToCpu::kNo);

    void preCompile(const PaintCombinations&);

    // Provides access to functions that aren't part of the public API.
    ContextPriv priv();
    const ContextPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    Context(sk_sp<Gpu>);

private:
    friend class ContextPriv;

    std::vector<std::unique_ptr<Recording>> fRecordings;
    sk_sp<Gpu> fGpu;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED
