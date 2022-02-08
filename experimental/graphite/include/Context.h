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
#include "include/private/SkPaintParamsKey.h"

#include "src/shaders/SkImageShader.h"

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/src/EnumBitMask.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkKeyHelpers.h"

#include "experimental/graphite/include/Combinations.h"

namespace skgpu {

class BackendTexture;
class ContextPriv;
class GlobalCache;
class Gpu;
class Recorder;
class Recording;
class TextureInfo;
namespace mtl { struct BackendContext; }

/**************************************************************************************************/
enum CombinableBlendMode : uint8_t {
    kNone    = 0x0,
    kClear   = 0x1,
    kSrc     = 0x2,
    kDst     = 0x4,
    kSrcOver = 0x8,

    kLast2 = kSrcOver
};
static const int kBlendModeCount = 4;

SkBlendMode CombinableBMtoBM(CombinableBlendMode combinableBM);
int FirstCombinableBlendMode(CombinableBlendMode combinableBlendModes);

/**/
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

    virtual void addToCode(SkShaderCodeDictionary* dict,
                           const Iter&,
                           SkBackend,
                           SkPaintParamsKey*) const = 0;

private:
};

class NoShaderCombo : public ShaderCombo2 {
public:
    ~NoShaderCombo() override {}

    Iter begin() const override { return { false, {0, 0, 0} }; }
    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);
        *iter = Iter::kEnd;
    }

    void addToCode(SkShaderCodeDictionary* dict,
                   const Iter& iter,
                   SkBackend backend,
                   SkPaintParamsKey* key) const override {
        SkASSERT(!iter.fDone);

        SolidColorShaderBlock::AddToKey(dict, backend, key, nullptr, SkColors::kRed);
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

    void addToCode(SkShaderCodeDictionary*,
                   const Iter&,
                   SkBackend,
                   SkPaintParamsKey*) const override;

private:
    CombinableTileMode fTileModes[2];
};

class GradientCombinations : public ShaderCombo2 {
public:
    static const int kGradTypeShift = 0;
//    static const int kTileModeShift = kGradientCodeBits;
//    static const int kNumStopsShift = kGradientCodeBits + kTileModeBits;

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

    Iter begin() const override {
        return { false, { FirstCombinableGradientType(fCombinableGradientTypes),
                          FirstCombinableTileMode(fCombinableTileModes),
                          (int) fMinNumStops} };
    }

    void next(Iter* iter) const override {
        SkASSERT(!iter->fDone);

        bool first = true;
        for (; iter->fIndices[0] <= (int) CombinableGradientType::kLast; iter->fIndices[0] <<= 1) {
            if (!(iter->fIndices[0] & (int) fCombinableGradientTypes)) {
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

    void addToCode(SkShaderCodeDictionary*,
                   const Iter&,
                   SkBackend,
                   SkPaintParamsKey*) const override;

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

    void getCode(SkShaderCodeDictionary*,
                 const Iter&,
                 SkBackend,
                 SkPaintParamsKey*) const;

private:
    std::vector<std::unique_ptr<ShaderCombo2>> fShaderCombos;
//    std::vector<ImageFilterCombo> fIFCombos;
    CombinableBlendMode fCombinableBlendModes = CombinableBlendMode::kNone;
};

class Context final {
public:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    ~Context();

#ifdef SK_METAL
    static std::unique_ptr<Context> MakeMetal(const skgpu::mtl::BackendContext&);
#endif

    BackendApi backend() const { return fBackend; }

    std::unique_ptr<Recorder> makeRecorder();

    void insertRecording(std::unique_ptr<Recording>);
    void submit(SyncToCpu = SyncToCpu::kNo);

    void preCompile(const PaintCombinations&);

    /**
     * Creates a new backend gpu texture matching the dimensinos and TextureInfo. If an invalid
     * TextureInfo or a TextureInfo Skia can't support is passed in, this will return an invalid
     * BackendTexture. Thus the client should check isValid on the returned BackendTexture to know
     * if it succeeded or not.
     *
     * If this does return a valid BackendTexture, the caller is required to use
     * Context::deleteBackendTexture to delete that texture.
     */
    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);

    /**
     * Called to delete the passed in BackendTexture. This should only be called if the
     *
     * BackendTexture was created by calling Context::createBackendTexture. If the BackendTexture is
     * not valid or does not match the BackendApi of the Context then nothing happens.
     *
     * Otherwise this will delete/release the backend object that is wrapped in the BackendTexture.
     * The BackendTexture will be reset to an invalid state and should not be used again.
     */
    void deleteBackendTexture(BackendTexture&);

    // Provides access to functions that aren't part of the public API.
    ContextPriv priv();
    const ContextPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    Context(sk_sp<Gpu>, BackendApi);

private:
    friend class ContextPriv;

    std::vector<std::unique_ptr<Recording>> fRecordings;
    sk_sp<Gpu> fGpu;
    sk_sp<GlobalCache> fGlobalCache;
    BackendApi fBackend;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED
