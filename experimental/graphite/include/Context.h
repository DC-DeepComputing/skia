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

/**/
inline static constexpr int kTileModeBits = 2;
inline static constexpr int kTileModeMask = 0x3;

enum CombinableTileMode : uint8_t {
//    kNone   = 0x0,
    kClamp  = 0x1,
    kRepeat = 0x2,
    kMirror = 0x4,
    kDecal  = 0x8,

    kLast = kDecal,
};
//GR_MAKE_BITFIELD_CLASS_OPS(CombinableTileMode)

/**/
//inline static constexpr int kCombinableTileModeBits = 2;
inline static constexpr int kCombinableTileModeCount = 4;
inline static constexpr CombinableTileMode kAllCombinableTileModes =
        (CombinableTileMode) (CombinableTileMode::kClamp  |
                              CombinableTileMode::kRepeat |
                              CombinableTileMode::kMirror |
                              CombinableTileMode::kDecal);
//inline static constexpr int kCombinableTileModeMask = 0xF;

static SkTileMode CombinableTMToTM(CombinableTileMode tm) {
    static_assert((0x1 << (kCombinableTileModeCount-1)) == (int) CombinableTileMode::kLast);

    switch (tm) {
        case CombinableTileMode::kClamp:  return SkTileMode::kClamp;
        case CombinableTileMode::kRepeat: return SkTileMode::kRepeat;
        case CombinableTileMode::kMirror: return SkTileMode::kMirror;
        case CombinableTileMode::kDecal:  return SkTileMode::kDecal;
    }

    SkASSERT(0);
    return SkTileMode::kRepeat;
}

#if 0
static CombinableTileMode TMToCombinableTM(SkTileMode tm) {
    static_assert(CombinableTileMode::kClamp == (0x1 << (int) SkTileMode::kClamp));
    static_assert(CombinableTileMode::kRepeat == (0x1 << (int) SkTileMode::kRepeat));
    static_assert(CombinableTileMode::kMirror == (0x1 << (int) SkTileMode::kMirror));
    static_assert(CombinableTileMode::kDecal == (0x1 << (int) SkTileMode::kDecal));

    return (CombinableTileMode) (0x1 << (int)tm);
}
#endif

static const char* TMToStr(SkTileMode tm) {
    const char* strings[4] = { "clamp", "repeat", "mirror", "decal" };
    return strings[(int) tm];
}

static int FirstCombinableTileMode(CombinableTileMode combinableTileMode) {
    int initialTM = 0x1;
    for ( ; initialTM <= (int) CombinableTileMode::kLast; initialTM <<= 1) {
        if (combinableTileMode & initialTM) {
            break;
        }
    }

    SkASSERT(initialTM <= (int) CombinableTileMode::kLast);
    return initialTM;
}
/**/

enum class GradientCode : uint8_t {
    kLinear,
    kRadial,
    kConical,
    kSweep,

    kLast = kSweep,
};
inline static constexpr int kGradientCodeBits = 2;
inline static constexpr int kGradientCodeMask = 0x3;

enum CombinableGradientType : uint8_t {
    kLinear  = 0x01,
    kRadial  = 0x02,
    kConical = 0x04,
    kSweep   = 0x08,

    kLast1 = kSweep,
};
//static const int kGradientTypeBits = 4;
static const int kCombinableGradientTypeCount = 4;
static const CombinableGradientType kAllCombinableGradientTypes =
        (CombinableGradientType) (CombinableGradientType::kLinear  |
                                  CombinableGradientType::kRadial  |
                                  CombinableGradientType::kConical |
                                  CombinableGradientType::kSweep);
//static const int kGradientTypeMask = 0xF;

static GradientCode CombinableGTToGT(CombinableGradientType type) {
    static_assert((0x1 << (kCombinableGradientTypeCount-1)) == CombinableGradientType::kLast1);

    switch (type) {
        case CombinableGradientType::kLinear:  return GradientCode::kLinear;
        case CombinableGradientType::kRadial:  return GradientCode::kRadial;
        case CombinableGradientType::kConical: return GradientCode::kConical;
        case CombinableGradientType::kSweep:   return GradientCode::kSweep;
    }

    SkASSERT(0);
    return GradientCode::kLinear;
}

#if 0
static GradientType CodeToGradType(int gradCode) {
    SkASSERT(gradCode >= 0 && gradCode < 4);
    GradientType types[4] = { kLinear_GradientType, kRadial_GradientType,
                              kConical_GradientType, kSweep_GradientType };
    return types[gradCode];
}
#endif

static const char* GTToStr(GradientCode gt) {
    const char* strings[4] = { "linear", "radial", "conical", "sweep" };
    return strings[(int) gt];
}

static int FirstCombinableGradientType(CombinableGradientType combinableGradientType) {
    int initialGT = 0x1;
    for ( ; initialGT <= (int) CombinableGradientType::kLast1; initialGT <<= 1) {
        if (combinableGradientType & initialGT) {
            break;
        }
    }

    SkASSERT(initialGT <= (int) CombinableGradientType::kLast1);
    return initialGT;
}
/**/
enum CombinableBlendMode : uint8_t {
    kNone    = 0x0,
    kClear   = 0x1,
    kSrc     = 0x2,
    kDst     = 0x4,
    kSrcOver = 0x8,

    kLast2 = kSrcOver
};
static const int kBlendModeBits = 4;
static const int kBlendModeCount = 4;

SkBlendMode CombinableBMtoBM(CombinableBlendMode combinableBM);
const char* BMToStr(SkBlendMode bm);
int FirstCombinableBlendMode(CombinableBlendMode combinableBlendModes);

/**/

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
static const int kShaderCodeBits = 8;
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
    ImageShaderCombinations() : fTileModes { kAllCombinableTileModes, kAllCombinableTileModes } {}

    ~ImageShaderCombinations() override {}

    Iter begin() const override {
        return { false,
                 { FirstCombinableTileMode(fTileModes[0]),
                   FirstCombinableTileMode(fTileModes[1]),
                   0} };
    }
    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);

        bool first = true;
        for (; iter->fIndices[0] <= (int) CombinableTileMode::kLast; iter->fIndices[0] <<= 1) {
            if (!(iter->fIndices[0] & (int) fTileModes[0])) {
                continue;
            }

            for (; iter->fIndices[1] <= (int) CombinableTileMode::kLast; iter->fIndices[1] <<= 1) {
                if (!(iter->fIndices[1] & (int)fTileModes[1])) {
                    continue;
                }

                if (first) {
                    first = false;
                    continue;
                }

                // TODO: iter is valid
                return;
            }

            iter->fIndices[1] = FirstCombinableTileMode(fTileModes[1]);
        }

        *iter = Iter::kEnd;
    }

    void addToCode(const Iter& iter, Combo2* combo) const override {
        SkASSERT(!iter.fDone);

        int code = Encode({ CombinableTMToTM((CombinableTileMode) iter.fIndices[0]),
                            CombinableTMToTM((CombinableTileMode) iter.fIndices[1]) });
        combo->addInt(code);
    }

    static void Dump(const Combo2& combo) {
        ImageData data = Decode(combo.fInts[0]);

        SkDebugf("imageShader(%s, %s)", TMToStr(data.fTileModes[0]), TMToStr(data.fTileModes[1]));
    }

private:
    struct ImageData {
        bool operator==(const ImageData& rhs) const {
            return fTileModes[0] == rhs.fTileModes[0] &&
                   fTileModes[1] == rhs.fTileModes[1];
        }
        bool operator!=(const ImageData& rhs) const { return !(*this == rhs); }

        SkTileMode fTileModes[2];
    };

    static const int kShaderCodeShift = 0;
    static const int kXTileModeShift = kShaderCodeBits;
    static const int kYTileModeShift = kShaderCodeBits + kTileModeBits;

    static ImageData Decode(int code) {
        int shaderCode = (code >> kShaderCodeShift) & kShaderCodeMask;
        SkTileMode tmX = (SkTileMode) (((code) >> kXTileModeShift) & kTileModeMask);
        SkTileMode tmY = (SkTileMode) (((code) >> kYTileModeShift) & kTileModeMask);

        SkASSERT(shaderCode == kImage_ShaderCode);
        return { tmX, tmY };
    }

    static int Encode(ImageData data) {
        int code = (kImage_ShaderCode << kShaderCodeShift) |
                   ((int) data.fTileModes[0] << kXTileModeShift) |
                   ((int) data.fTileModes[1] << kYTileModeShift);
        SkASSERT(data == Decode(code));
        return code;
    }

    CombinableTileMode fTileModes[2];
};

class GradientCombinations : public ShaderCombo2 {
public:
    static const int kShaderCodeShift = 0;
    static const int kGradTypeShift = kShaderCodeBits;
    static const int kTileModeShift = kShaderCodeBits + kGradientCodeBits;
    static const int kNumStopsShift = kShaderCodeBits + kGradientCodeBits + kTileModeBits;

    static const int kNumStopsMask = 0x1FF;

    GradientCombinations(CombinableGradientType types = kAllCombinableGradientTypes,
                         CombinableTileMode tileModes = kAllCombinableTileModes,
                         unsigned int minNumStops = 2,
                         unsigned int maxNumStops = 256)
            : fCombinableGradientTypes(types)
            , fCombinableTileModes(tileModes)
            , fMinNumStops(minNumStops)
            , fMaxNumStops(maxNumStops) {}
    ~GradientCombinations() override {}

    struct GradData {
        bool operator==(const GradData& rhs) const {
            return fType == rhs.fType &&
                   fTM == rhs.fTM &&
                   fNumStops == rhs.fNumStops;
        }
        bool operator!=(const GradData& rhs) const { return !(*this == rhs); }

        GradientCode fType;
        SkTileMode   fTM;
        int          fNumStops;
    };

    static GradData Decode(int code) {
        ShaderCode shaderCode = (ShaderCode) ((code >> kShaderCodeShift) & kShaderCodeMask);
        GradientCode gradType = (GradientCode) ((code >> kGradTypeShift) & kGradientCodeMask);
        SkTileMode tm  = (SkTileMode) ((code >> kTileModeShift) & kTileModeMask);
        int numStops   = (code >> kNumStopsShift) & kNumStopsMask;

        SkASSERT(shaderCode == kGradient_ShaderCode);
        return { gradType, tm, numStops };
    }

    static int Encode(GradData data) {
//        int typeCode = GradTypeToCode(data.fType);

//        SkASSERT((typeCode & kGradientTypeMask) == typeCode);
//        SkASSERT((tileModeCode & kCombinableTileModeMask) == tileModeCode);

        int code = (kGradient_ShaderCode << kShaderCodeShift) |
                   ((int)data.fType << kGradTypeShift) |
                   ((int) data.fTM << kTileModeShift) |
                   data.fNumStops << kNumStopsShift;
        SkDEBUGCODE(GradData newData = Decode(code);)
        SkASSERT(data == newData);
        return code;
    }

    Iter begin() const override {
        return { false, { FirstCombinableGradientType(fCombinableGradientTypes),
                          FirstCombinableTileMode(fCombinableTileModes),
                          (int) fMinNumStops} };
    }

    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);

        bool first = true;
        for (; iter->fIndices[0] <= CombinableGradientType::kLast1; iter->fIndices[0] <<= 1) {
            if (!(iter->fIndices[0] & fCombinableGradientTypes)) {
                continue;
            }

            for (; iter->fIndices[1] <= (int) CombinableTileMode::kLast; iter->fIndices[1] <<= 1) {
                if (!(iter->fIndices[1] & (int) fCombinableTileModes)) {
                    continue;
                }

                for (; iter->fIndices[2] <= (int) fMaxNumStops; ++iter->fIndices[2]) {
                    if (first) {
                        first = false;
                        continue;
                    }

                    return;
                }

                iter->fIndices[2] = fMinNumStops;
            }

            iter->fIndices[1] = FirstCombinableTileMode(fCombinableTileModes);
        }

        *iter = Iter::kEnd;
    }

    void addToCode(const Iter& iter, Combo2* combo) const override {
        SkASSERT(!iter.fDone);
        int code = Encode({ CombinableGTToGT((CombinableGradientType) iter.fIndices[0]),
                            CombinableTMToTM((CombinableTileMode) iter.fIndices[1]),
                            iter.fIndices[2] });
        combo->addInt(code);
    }

    static void Dump(const Combo2& combo) {
        GradData data = Decode(combo.fInts[0]);

        SkDebugf("gradientShader(%s, %s, %d)", GTToStr(data.fType), TMToStr(data.fTM), data.fNumStops);
    }

private:
    CombinableGradientType fCombinableGradientTypes;
    CombinableTileMode fCombinableTileModes;
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

class PaintCombinations {
public:
    void addShaderCombinations(std::unique_ptr<ShaderCombo2> combo) {
        fShaderCombos.emplace_back(std::move(combo));
    }
    void addBlendModeCombinations(CombinableBlendMode combinableBlendModes) {
        fCombinableBlendModes = (CombinableBlendMode) (fCombinableBlendModes | combinableBlendModes);
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
    CombinableBlendMode fCombinableBlendModes = CombinableBlendMode::kNone;
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
