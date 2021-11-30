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
    static const int kMaxInts = 8;

    void addInt(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fInts[fNumInts] = iFoo;
        ++fNumInts;
    }

    int numInts() const { return fNumInts; }

    void dump() const;

//private:
    int fNumInts = 0;
    std::array<int, kMaxInts> fInts;
};

class ShaderCombo2 {
public:
    virtual ~ShaderCombo2() {}

    class Iter {
    public:
        bool operator==(const Iter& rhs) const { return fDone == rhs.fDone &&
                                                        fIndices == rhs.fIndices; }
        bool operator!=(const Iter& rhs) const { return !(*this == rhs); }

        bool fDone;
        std::array<int, 3> fIndices;

        static const Iter kEnd;
    };

    virtual Iter begin() const = 0;
    Iter end() const { return Iter::kEnd; }

    virtual void next(Iter*) const = 0;

    virtual void addToCode(const Iter&, Combo2*) const = 0;

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

enum ShaderCode {
    kNone_ShaderCode = 1,
    kImage_ShaderCode = 2,
    kGradient_ShaderCode = 3,
};
static const int kShaderCodeBits = 16;
static const int kShaderCodeMask = 0xFF;

class NoShaderCombo : public ShaderCombo2 {
public:
    ~NoShaderCombo() override {}

    Iter begin() const override { return { false, {0, 0, 0} }; }
    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);
        *iter = Iter::kEnd;
    }

    void addToCode(const Iter& iter, Combo2* combo) const override {
        SkASSERT(!iter.fDone);
        combo->addInt(kNone_ShaderCode);
    }

    static void Dump(const Combo2& combo) {
        SkASSERT(combo.fInts[0] == kNone_ShaderCode);
        SkDebugf("noShader");
    }

private:
};

class ImageShaderCombinations : public ShaderCombo2 {
public:
    ~ImageShaderCombinations() override {}

    Iter begin() const override { return { false, {0, 0, 0} };  }
    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);
        *iter = Iter::kEnd;
    }
    void addToCode(const Iter& iter, Combo2* combo) const override {
        SkASSERT(!iter.fDone);
        combo->addInt(kImage_ShaderCode);
    }

    static void Dump(const Combo2& combo) {
        SkASSERT(combo.fInts[0] == kImage_ShaderCode);
        SkDebugf("imageShader");
    }

private:
};

class GradientCombinations : public ShaderCombo2 {
public:
    enum GradientType {
        kLinear_GradientType  = 0x01,
        kRadial_GradientType  = 0x02,
        kConical_GradientType = 0x04,
        kSweep_GradientType   = 0x08,

        kLast_GradientType = kSweep_GradientType,
    };
    static const int kGradientTypeBits = 2;
    static const int kGradientTypeCount = 4;
    static const int kAllGradientTypes = kLinear_GradientType |
                                         kRadial_GradientType |
                                         kConical_GradientType |
                                         kSweep_GradientType;
    static const int kGradientTypeMask = 0xF;

    enum TileMode {
        kClamp_TileMode  = 0x01,
        kRepeat_TileMode = 0x02,
        kMirror_TileMode = 0x04,
        kDecal_TileMode  = 0x08,

        kLast_TileMode = kDecal_TileMode,
    };
    static const int kTileModeBits = 2;
    static const int kTileModeCount = 4;
    static const int kAllTileModes = kClamp_TileMode |
                                     kRepeat_TileMode |
                                     kMirror_TileMode |
                                     kDecal_TileMode;
    static const int kTileModeMask = 0xF;

    static const int kShaderCodeShift = 0;
    static const int kGradTypeShift = kShaderCodeBits;
    static const int kTileModeShift = kShaderCodeBits + kGradientTypeBits;

    GradientCombinations(GradientType types = (GradientType) kAllGradientTypes)
            : fTypes(types)
            , fTileModes((TileMode) kAllTileModes)
            , fMinNumStops(2)
            , fMaxNumStops(256) {}
    GradientCombinations(GradientType types, unsigned int minNumStops, unsigned int maxNumStops = 256)
            : fTypes(types)
            , fMinNumStops(minNumStops)
            , fMaxNumStops(maxNumStops) {}
    GradientCombinations(unsigned int minNumStops, unsigned int maxNumStops = 256)
            : fTypes((GradientType) kAllGradientTypes)
            , fTileModes((TileMode) kAllTileModes)
            , fMinNumStops(minNumStops)
            , fMaxNumStops(maxNumStops) {}
    ~GradientCombinations() override {}

    static int GradTypeToCode(GradientType type) {
        static_assert((0x1 << (kGradientTypeCount-1)) == kLast_GradientType);

        int code = 0;
        switch (type) {
            case kLinear_GradientType:  code = 0; break;
            case kRadial_GradientType:  code = 1; break;
            case kConical_GradientType: code = 2; break;
            case kSweep_GradientType:   code = 3; break;
        }

        SkASSERT(code < kGradientTypeCount);
        return code;
    }

    static GradientType CodeToGradType(int gradCode) {
        SkASSERT(gradCode >= 0 && gradCode < 4);
        GradientType types[4] = { kLinear_GradientType, kRadial_GradientType,
                                  kConical_GradientType, kSweep_GradientType };
        return types[gradCode];
    }

    static const char* GradTypeToStr(GradientType type) {
        int typeCode = GradTypeToCode(type);
        SkASSERT(typeCode >= 0 && typeCode < 4);
        const char* strings[4] = { "linear", "radial", "conical", "sweep" };
        return strings[typeCode];
    }

    static int TileModeToCode(TileMode tm) {
        static_assert((0x1 << (kTileModeCount-1)) == kLast_TileMode);

        int code = 0;
        switch (tm) {
            case kClamp_TileMode:  code = 0; break;
            case kRepeat_TileMode: code = 1; break;
            case kMirror_TileMode: code = 2; break;
            case kDecal_TileMode:  code = 3; break;
        }

        SkASSERT(code < kTileModeCount);
        return code;
    }

    static TileMode CodeToTileMode(int tmCode) {
        SkASSERT(tmCode >= 0 && tmCode < 4);
        TileMode modes[4] = { kClamp_TileMode, kRepeat_TileMode, kMirror_TileMode, kDecal_TileMode };
        return modes[tmCode];
    }

    static const char* TileModeToStr(TileMode tm) {
        int tmCode = TileModeToCode(tm);
        SkASSERT(tmCode >= 0 && tmCode < 4);
        const char* strings[4] = { "clamp", "repeat", "mirror", "decal" };
        return strings[tmCode];
    }

    struct GradData {
//        bool operator==(const GradData&) const = default;

        GradientType fType;
        TileMode     fTM;
        int          fNumStops;
    };

    static GradData Decode(int code) {
        int shaderCode = (code >> kShaderCodeBits) & kShaderCodeMask;
        int typeCode   = (code >> kGradTypeShift) & kGradientTypeMask;
        int tmCode     = (code >> kTileModeShift) & kTileModeMask;

        SkASSERT(shaderCode == kGradient_ShaderCode);
        return { CodeToGradType(typeCode), CodeToTileMode(tmCode), 0};
    }

    static int Encode(GradData data) {
        int typeCode = GradTypeToCode(data.fType);
        int tileModeCode = TileModeToCode(data.fTM);

        SkASSERT((typeCode & kGradientTypeMask) == typeCode);
        SkASSERT((tileModeCode & kTileModeMask) == tileModeCode);

        int code = (kGradient_ShaderCode << kShaderCodeBits) |
                   (typeCode << kGradTypeShift) |
                   (tileModeCode << kTileModeShift);
//        SkASSERT(data == Decode(code));
        return code;
    }

    Iter begin() const override {
        return { false, {0x1, 0x1, (int) fMinNumStops} };
    }

    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);

        for (; iter->fIndices[0] <= kLast_GradientType; iter->fIndices[0] <<= 1) {
            if (!(iter->fIndices[0] & fTypes)) {
                continue;
            }

            for (; iter->fIndices[1] <= kLast_TileMode; iter->fIndices[1] <<= 1) {
                if (!(iter->fIndices[1] & fTileModes)) {
                    continue;
                }

                for (; iter->fIndices[2] <= (int) fMaxNumStops; ++iter->fIndices[2]) {
                    return;
                }
            }
        }

        *iter = Iter::kEnd;
    }

    void addToCode(const Iter& iter, Combo2* combo) const override {
        SkASSERT(!iter.fDone);
        int code = Encode({ (GradientType) iter.fIndices[0],
                            (TileMode) iter.fIndices[1],
                            iter.fIndices[2] });
        combo->addInt(code);
    }

    static void Dump(const Combo2& combo) {
        GradData data = Decode(combo.fInts[0]);

        SkDebugf("gradientShader(%s, %s)", GradTypeToStr(data.fType), TileModeToStr(data.fTM));
    }

private:
    GradientType fTypes;
    TileMode fTileModes;
    unsigned int fMinNumStops;
    unsigned int fMaxNumStops;
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
};
#endif

enum BlendModeCombinations {
    kNone    = 0x0,
    kClear   = 0x1,
    kSrc     = 0x2,
    kDst     = 0x4,
    kSrcOver = 0x8,

    kLast_BlendModes = kSrcOver
};
static const int kBlendModeBits = 4;
static const int kBlendModeCount = 4;

class PaintCombinations {
public:
    void addShaderCombinations(std::unique_ptr<ShaderCombo2> combo) {
        fShaderCombos.emplace_back(std::move(combo));
    }
    void addBlendModeCombinations(BlendModeCombinations blendModeCombinations) {
        fBlendModes = (BlendModeCombinations) (fBlendModes | blendModeCombinations);
    }

    class Iter {
    public:
        bool operator==(const Iter& rhs) const { return fDone == rhs.fDone &&
                                                        fIndices == rhs.fIndices; }
        bool operator!=(const Iter& rhs) const { return !(*this == rhs); }

        bool fDone;
        std::array<int, 2> fIndices;  // 0 -> shader index; 1 -> blend mode

        ShaderCombo2* fLastShaderCombo = nullptr;
        ShaderCombo2::Iter fShaderIter = ShaderCombo2::Iter::kEnd;

        static const Iter kEnd;
    };

    Iter begin() const;
    Iter end() const { return Iter::kEnd; }

    void next(Iter*) const;
    SkDEBUGCODE(void checkIter(const Iter&) const;)

    void getCode(const Iter&, Combo2*) const;

private:
    std::vector<std::unique_ptr<ShaderCombo2>> fShaderCombos;
//    std::vector<ImageFilterCombo> fIFCombos;
    BlendModeCombinations fBlendModes = BlendModeCombinations::kNone;
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

    static void preCompile(const PaintCombinations&);

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
