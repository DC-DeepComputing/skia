/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkPerlinNoiseShader.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GANESH)
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#endif // SK_GANESH

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/image/SkImage_Base.h"
#endif // SK_GRAPHITE

static const int kBlockSize = 256;
static const int kBlockMask = kBlockSize - 1;
static const int kPerlinNoise = 4096;
static const int kRandMaximum = SK_MaxS32; // 2**31 - 1

class SkPerlinNoiseShader : public SkShaderBase {
public:
    struct StitchData {
        StitchData()
          : fWidth(0)
          , fWrapX(0)
          , fHeight(0)
          , fWrapY(0)
        {}

        StitchData(SkScalar w, SkScalar h)
          : fWidth(std::min(SkScalarRoundToInt(w), SK_MaxS32 - kPerlinNoise))
          , fWrapX(kPerlinNoise + fWidth)
          , fHeight(std::min(SkScalarRoundToInt(h), SK_MaxS32 - kPerlinNoise))
          , fWrapY(kPerlinNoise + fHeight) {}

        bool operator==(const StitchData& other) const {
            return fWidth == other.fWidth &&
                   fWrapX == other.fWrapX &&
                   fHeight == other.fHeight &&
                   fWrapY == other.fWrapY;
        }

        int fWidth; // How much to subtract to wrap for stitching.
        int fWrapX; // Minimum value to wrap.
        int fHeight;
        int fWrapY;
    };

    struct PaintingData {
        PaintingData(const SkISize& tileSize, SkScalar seed,
                     SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                     const SkMatrix& matrix)
        {
            SkVector tileVec;
            matrix.mapVector(SkIntToScalar(tileSize.fWidth), SkIntToScalar(tileSize.fHeight),
                             &tileVec);

            SkSize scale;
            if (!matrix.decomposeScale(&scale, nullptr)) {
                scale.set(SK_ScalarNearlyZero, SK_ScalarNearlyZero);
            }
            fBaseFrequency.set(baseFrequencyX * SkScalarInvert(scale.width()),
                               baseFrequencyY * SkScalarInvert(scale.height()));
            fTileSize.set(SkScalarRoundToInt(tileVec.fX), SkScalarRoundToInt(tileVec.fY));
            this->init(seed);
            if (!fTileSize.isEmpty()) {
                this->stitch();
            }

    #if defined(SK_GANESH) || defined(SK_GRAPHITE)
            SkImageInfo info = SkImageInfo::MakeA8(kBlockSize, 1);
            fPermutationsBitmap.installPixels(info, fLatticeSelector, info.minRowBytes());
            fPermutationsBitmap.setImmutable();

            info = SkImageInfo::Make(kBlockSize, 4, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            fNoiseBitmap.installPixels(info, fNoise[0][0], info.minRowBytes());
            fNoiseBitmap.setImmutable();
    #endif
        }

    #if defined(SK_GANESH) || defined(SK_GRAPHITE)
        PaintingData(const PaintingData& that)
                : fSeed(that.fSeed)
                , fTileSize(that.fTileSize)
                , fBaseFrequency(that.fBaseFrequency)
                , fStitchDataInit(that.fStitchDataInit)
                , fPermutationsBitmap(that.fPermutationsBitmap)
                , fNoiseBitmap(that.fNoiseBitmap) {
            memcpy(fLatticeSelector, that.fLatticeSelector, sizeof(fLatticeSelector));
            memcpy(fNoise, that.fNoise, sizeof(fNoise));
            memcpy(fGradient, that.fGradient, sizeof(fGradient));
        }
    #endif

        int         fSeed;
        uint8_t     fLatticeSelector[kBlockSize];
        uint16_t    fNoise[4][kBlockSize][2];
        SkPoint     fGradient[4][kBlockSize];
        SkISize     fTileSize;
        SkVector    fBaseFrequency;
        StitchData  fStitchDataInit;

    private:

    #if defined(SK_GANESH) || defined(SK_GRAPHITE)
        SkBitmap fPermutationsBitmap;
        SkBitmap fNoiseBitmap;
    #endif

        inline int random()  {
            // See https://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
            // m = kRandMaximum, 2**31 - 1 (2147483647)
            static constexpr int kRandAmplitude = 16807; // 7**5; primitive root of m
            static constexpr int kRandQ = 127773; // m / a
            static constexpr int kRandR = 2836; // m % a

            int result = kRandAmplitude * (fSeed % kRandQ) - kRandR * (fSeed / kRandQ);
            if (result <= 0) {
                result += kRandMaximum;
            }
            fSeed = result;
            return result;
        }

        // Only called once. Could be part of the constructor.
        void init(SkScalar seed) {
            // According to the SVG spec, we must truncate (not round) the seed value.
            fSeed = SkScalarTruncToInt(seed);
            // The seed value clamp to the range [1, kRandMaximum - 1].
            if (fSeed <= 0) {
                fSeed = -(fSeed % (kRandMaximum - 1)) + 1;
            }
            if (fSeed > kRandMaximum - 1) {
                fSeed = kRandMaximum - 1;
            }
            for (int channel = 0; channel < 4; ++channel) {
                for (int i = 0; i < kBlockSize; ++i) {
                    fLatticeSelector[i] = i;
                    fNoise[channel][i][0] = (random() % (2 * kBlockSize));
                    fNoise[channel][i][1] = (random() % (2 * kBlockSize));
                }
            }
            for (int i = kBlockSize - 1; i > 0; --i) {
                int k = fLatticeSelector[i];
                int j = random() % kBlockSize;
                SkASSERT(j >= 0);
                SkASSERT(j < kBlockSize);
                fLatticeSelector[i] = fLatticeSelector[j];
                fLatticeSelector[j] = k;
            }

            // Perform the permutations now
            {
                // Copy noise data
                uint16_t noise[4][kBlockSize][2];
                for (int i = 0; i < kBlockSize; ++i) {
                    for (int channel = 0; channel < 4; ++channel) {
                        for (int j = 0; j < 2; ++j) {
                            noise[channel][i][j] = fNoise[channel][i][j];
                        }
                    }
                }
                // Do permutations on noise data
                for (int i = 0; i < kBlockSize; ++i) {
                    for (int channel = 0; channel < 4; ++channel) {
                        for (int j = 0; j < 2; ++j) {
                            fNoise[channel][i][j] = noise[channel][fLatticeSelector[i]][j];
                        }
                    }
                }
            }

            // Half of the largest possible value for 16 bit unsigned int
            static constexpr SkScalar kHalfMax16bits = 32767.5f;

            // Compute gradients from permuted noise data
            static constexpr SkScalar kInvBlockSizef = 1.0 / SkIntToScalar(kBlockSize);
            for (int channel = 0; channel < 4; ++channel) {
                for (int i = 0; i < kBlockSize; ++i) {
                    fGradient[channel][i] = SkPoint::Make(
                        (fNoise[channel][i][0] - kBlockSize) * kInvBlockSizef,
                        (fNoise[channel][i][1] - kBlockSize) * kInvBlockSizef);
                    fGradient[channel][i].normalize();
                    // Put the normalized gradient back into the noise data
                    fNoise[channel][i][0] =
                            SkScalarRoundToInt((fGradient[channel][i].fX + 1) * kHalfMax16bits);
                    fNoise[channel][i][1] =
                            SkScalarRoundToInt((fGradient[channel][i].fY + 1) * kHalfMax16bits);
                }
            }
        }

        // Only called once. Could be part of the constructor.
        void stitch() {
            SkScalar tileWidth  = SkIntToScalar(fTileSize.width());
            SkScalar tileHeight = SkIntToScalar(fTileSize.height());
            SkASSERT(tileWidth > 0 && tileHeight > 0);
            // When stitching tiled turbulence, the frequencies must be adjusted
            // so that the tile borders will be continuous.
            if (fBaseFrequency.fX) {
                SkScalar lowFrequencx =
                    SkScalarFloorToScalar(tileWidth * fBaseFrequency.fX) / tileWidth;
                SkScalar highFrequencx =
                    SkScalarCeilToScalar(tileWidth * fBaseFrequency.fX) / tileWidth;
                // BaseFrequency should be non-negative according to the standard.
                // lowFrequencx can be 0 if fBaseFrequency.fX is very small.
                if (sk_ieee_float_divide(fBaseFrequency.fX, lowFrequencx) < highFrequencx / fBaseFrequency.fX) {
                    fBaseFrequency.fX = lowFrequencx;
                } else {
                    fBaseFrequency.fX = highFrequencx;
                }
            }
            if (fBaseFrequency.fY) {
                SkScalar lowFrequency =
                    SkScalarFloorToScalar(tileHeight * fBaseFrequency.fY) / tileHeight;
                SkScalar highFrequency =
                    SkScalarCeilToScalar(tileHeight * fBaseFrequency.fY) / tileHeight;
                // lowFrequency can be 0 if fBaseFrequency.fY is very small.
                if (sk_ieee_float_divide(fBaseFrequency.fY, lowFrequency) < highFrequency / fBaseFrequency.fY) {
                    fBaseFrequency.fY = lowFrequency;
                } else {
                    fBaseFrequency.fY = highFrequency;
                }
            }
            fStitchDataInit = StitchData(tileWidth * fBaseFrequency.fX,
                                         tileHeight * fBaseFrequency.fY);
        }

    public:

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
        const SkBitmap& getPermutationsBitmap() const { return fPermutationsBitmap; }

        const SkBitmap& getNoiseBitmap() const { return fNoiseBitmap; }
#endif
    };  // struct PaintingData

    /**
     *  About the noise types : the difference between the first 2 is just minor tweaks to the
     *  algorithm, they're not 2 entirely different noises. The output looks different, but once the
     *  noise is generated in the [1, -1] range, the output is brought back in the [0, 1] range by
     *  doing :
     *  kFractalNoise_Type : noise * 0.5 + 0.5
     *  kTurbulence_Type   : abs(noise)
     *  Very little differs between the 2 types, although you can tell the difference visually.
     */
    enum Type {
        kFractalNoise_Type,
        kTurbulence_Type,
        kLast_Type = kTurbulence_Type
    };

    static const int kMaxOctaves = 255; // numOctaves must be <= 0 and <= kMaxOctaves

    SkPerlinNoiseShader(SkPerlinNoiseShader::Type type, SkScalar baseFrequencyX,
                            SkScalar baseFrequencyY, int numOctaves, SkScalar seed,
                            const SkISize* tileSize);

    ShaderType type() const override { return ShaderType::kPerlinNoise; }


    class PerlinNoiseShaderContext : public Context {
    public:
        PerlinNoiseShaderContext(const SkPerlinNoiseShader& shader, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    private:
        SkPMColor shade(const SkPoint& point, StitchData& stitchData) const;
        SkScalar calculateTurbulenceValueForPoint(int channel,
                                                  StitchData& stitchData,
                                                  const SkPoint& point) const;
        SkScalar noise2D(int channel,
                         const StitchData& stitchData,
                         const SkPoint& noiseVector) const;

        SkMatrix     fMatrix;
        PaintingData fPaintingData;

        using INHERITED = Context;
    };

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif
#if defined(SK_ENABLE_SKVM)
    skvm::Color program(skvm::Builder*,
                        skvm::Coord,
                        skvm::Coord,
                        skvm::Color,
                        const SkShaders::MatrixRec&,
                        const SkColorInfo&,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override {
        // Unimplemented
        return {};
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkPerlinNoiseShader)

    const SkPerlinNoiseShader::Type fType;
    const SkScalar                  fBaseFrequencyX;
    const SkScalar                  fBaseFrequencyY;
    const int                       fNumOctaves;
    const SkScalar                  fSeed;
    const SkISize                   fTileSize;
    const bool                      fStitchTiles;

    friend void SkRegisterPerlinNoiseShaderFlattenable();

    using INHERITED = SkShaderBase;
};

namespace {

// noiseValue is the color component's value (or color)
// limitValue is the maximum perlin noise array index value allowed
// newValue is the current noise dimension (either width or height)
inline int checkNoise(int noiseValue, int limitValue, int newValue) {
    // If the noise value would bring us out of bounds of the current noise array while we are
    // stiching noise tiles together, wrap the noise around the current dimension of the noise to
    // stay within the array bounds in a continuous fashion (so that tiling lines are not visible)
    if (noiseValue >= limitValue) {
        noiseValue -= newValue;
    }
    return noiseValue;
}

inline SkScalar smoothCurve(SkScalar t) {
    return t * t * (3 - 2 * t);
}

} // end namespace

SkPerlinNoiseShader::SkPerlinNoiseShader(SkPerlinNoiseShader::Type type,
                                         SkScalar baseFrequencyX,
                                         SkScalar baseFrequencyY,
                                         int numOctaves,
                                         SkScalar seed,
                                         const SkISize* tileSize)
        : fType(type)
        , fBaseFrequencyX(baseFrequencyX)
        , fBaseFrequencyY(baseFrequencyY)
        , fNumOctaves(numOctaves > kMaxOctaves ? kMaxOctaves : numOctaves) //[0,255] octaves allowed
        , fSeed(seed)
        , fTileSize(nullptr == tileSize ? SkISize::Make(0, 0) : *tileSize)
        , fStitchTiles(!fTileSize.isEmpty()) {
    SkASSERT(numOctaves >= 0 && numOctaves <= kMaxOctaves);
    SkASSERT(fBaseFrequencyX >= 0);
    SkASSERT(fBaseFrequencyY >= 0);
}

sk_sp<SkFlattenable> SkPerlinNoiseShader::CreateProc(SkReadBuffer& buffer) {
    Type type = buffer.read32LE(kLast_Type);

    SkScalar freqX = buffer.readScalar();
    SkScalar freqY = buffer.readScalar();
    int octaves = buffer.read32LE<int>(kMaxOctaves);

    SkScalar seed = buffer.readScalar();
    SkISize tileSize;
    tileSize.fWidth = buffer.readInt();
    tileSize.fHeight = buffer.readInt();

    switch (type) {
        case kFractalNoise_Type:
            return SkShaders::MakeFractalNoise(freqX, freqY, octaves, seed, &tileSize);
        case kTurbulence_Type:
            return SkShaders::MakeTurbulence(freqX, freqY, octaves, seed, &tileSize);
        default:
            // Really shouldn't get here b.c. of earlier check on type
            buffer.validate(false);
            return nullptr;
    }
}

void SkPerlinNoiseShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int) fType);
    buffer.writeScalar(fBaseFrequencyX);
    buffer.writeScalar(fBaseFrequencyY);
    buffer.writeInt(fNumOctaves);
    buffer.writeScalar(fSeed);
    buffer.writeInt(fTileSize.fWidth);
    buffer.writeInt(fTileSize.fHeight);
}

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::noise2D(
        int channel, const StitchData& stitchData, const SkPoint& noiseVector) const {
    struct Noise {
        int noisePositionIntegerValue;
        int nextNoisePositionIntegerValue;
        SkScalar noisePositionFractionValue;
        Noise(SkScalar component)
        {
            SkScalar position = component + kPerlinNoise;
            noisePositionIntegerValue = SkScalarFloorToInt(position);
            noisePositionFractionValue = position - SkIntToScalar(noisePositionIntegerValue);
            nextNoisePositionIntegerValue = noisePositionIntegerValue + 1;
        }
    };
    Noise noiseX(noiseVector.x());
    Noise noiseY(noiseVector.y());
    SkScalar u, v;
    const SkPerlinNoiseShader& perlinNoiseShader = static_cast<const SkPerlinNoiseShader&>(fShader);
    // If stitching, adjust lattice points accordingly.
    if (perlinNoiseShader.fStitchTiles) {
        noiseX.noisePositionIntegerValue =
            checkNoise(noiseX.noisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.noisePositionIntegerValue =
            checkNoise(noiseY.noisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
        noiseX.nextNoisePositionIntegerValue =
            checkNoise(noiseX.nextNoisePositionIntegerValue, stitchData.fWrapX, stitchData.fWidth);
        noiseY.nextNoisePositionIntegerValue =
            checkNoise(noiseY.nextNoisePositionIntegerValue, stitchData.fWrapY, stitchData.fHeight);
    }
    noiseX.noisePositionIntegerValue &= kBlockMask;
    noiseY.noisePositionIntegerValue &= kBlockMask;
    noiseX.nextNoisePositionIntegerValue &= kBlockMask;
    noiseY.nextNoisePositionIntegerValue &= kBlockMask;
    int i = fPaintingData.fLatticeSelector[noiseX.noisePositionIntegerValue];
    int j = fPaintingData.fLatticeSelector[noiseX.nextNoisePositionIntegerValue];
    int b00 = (i + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b10 = (j + noiseY.noisePositionIntegerValue) & kBlockMask;
    int b01 = (i + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    int b11 = (j + noiseY.nextNoisePositionIntegerValue) & kBlockMask;
    SkScalar sx = smoothCurve(noiseX.noisePositionFractionValue);
    SkScalar sy = smoothCurve(noiseY.noisePositionFractionValue);

    if (sx < 0 || sy < 0 || sx > 1 || sy > 1) {
        return 0;  // Check for pathological inputs.
    }

    // This is taken 1:1 from SVG spec: http://www.w3.org/TR/SVG11/filters.html#feTurbulenceElement
    SkPoint fractionValue = SkPoint::Make(noiseX.noisePositionFractionValue,
                                          noiseY.noisePositionFractionValue); // Offset (0,0)
    u = fPaintingData.fGradient[channel][b00].dot(fractionValue);
    fractionValue.fX -= SK_Scalar1; // Offset (-1,0)
    v = fPaintingData.fGradient[channel][b10].dot(fractionValue);
    SkScalar a = SkScalarInterp(u, v, sx);
    fractionValue.fY -= SK_Scalar1; // Offset (-1,-1)
    v = fPaintingData.fGradient[channel][b11].dot(fractionValue);
    fractionValue.fX = noiseX.noisePositionFractionValue; // Offset (0,-1)
    u = fPaintingData.fGradient[channel][b01].dot(fractionValue);
    SkScalar b = SkScalarInterp(u, v, sx);
    return SkScalarInterp(a, b, sy);
}

SkScalar SkPerlinNoiseShader::PerlinNoiseShaderContext::calculateTurbulenceValueForPoint(
        int channel, StitchData& stitchData, const SkPoint& point) const {
    const SkPerlinNoiseShader& perlinNoiseShader = static_cast<const SkPerlinNoiseShader&>(fShader);
    if (perlinNoiseShader.fStitchTiles) {
        stitchData = fPaintingData.fStitchDataInit;
    }
    SkScalar turbulenceFunctionResult = 0;
    SkPoint noiseVector(SkPoint::Make(point.x() * fPaintingData.fBaseFrequency.fX,
                                      point.y() * fPaintingData.fBaseFrequency.fY));
    SkScalar ratio = SK_Scalar1;
    for (int octave = 0; octave < perlinNoiseShader.fNumOctaves; ++octave) {
        SkScalar noise = noise2D(channel, stitchData, noiseVector);
        SkScalar numer = (perlinNoiseShader.fType == kFractalNoise_Type) ?
                            noise : SkScalarAbs(noise);
        turbulenceFunctionResult += numer / ratio;
        noiseVector.fX *= 2;
        noiseVector.fY *= 2;
        ratio *= 2;
        if (perlinNoiseShader.fStitchTiles) {
            stitchData = StitchData(SkIntToScalar(stitchData.fWidth) * 2,
                                    SkIntToScalar(stitchData.fHeight) * 2);
        }
    }

    if (perlinNoiseShader.fType == kFractalNoise_Type) {
        // For kFractalNoise the result is: noise[-1,1] * 0.5 + 0.5
        turbulenceFunctionResult = SkScalarHalf(turbulenceFunctionResult + 1);
    }

    if (channel == 3) { // Scale alpha by paint value
        turbulenceFunctionResult *= SkIntToScalar(getPaintAlpha()) / 255;
    }

    // Clamp result
    return SkTPin(turbulenceFunctionResult, 0.0f, SK_Scalar1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SkPMColor SkPerlinNoiseShader::PerlinNoiseShaderContext::shade(
        const SkPoint& point, StitchData& stitchData) const {
    SkPoint newPoint;
    fMatrix.mapPoints(&newPoint, &point, 1);
    newPoint.fX = SkScalarRoundToScalar(newPoint.fX);
    newPoint.fY = SkScalarRoundToScalar(newPoint.fY);

    U8CPU rgba[4];
    for (int channel = 3; channel >= 0; --channel) {
        SkScalar value;
        value = calculateTurbulenceValueForPoint(channel, stitchData, newPoint);
        rgba[channel] = SkScalarFloorToInt(255 * value);
    }
    return SkPreMultiplyARGB(rgba[3], rgba[0], rgba[1], rgba[2]);
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkPerlinNoiseShader::onMakeContext(const ContextRec& rec,
                                                              SkArenaAlloc* alloc) const {
    // should we pay attention to rec's device-colorspace?
    return alloc->make<PerlinNoiseShaderContext>(*this, rec);
}
#endif

static inline SkMatrix total_matrix(const SkShaderBase::ContextRec& rec,
                                    const SkShaderBase& shader) {
    if (rec.fLocalMatrix) {
        return SkMatrix::Concat(*rec.fMatrix, *rec.fLocalMatrix);
    }
    return *rec.fMatrix;
}

SkPerlinNoiseShader::PerlinNoiseShaderContext::PerlinNoiseShaderContext(
        const SkPerlinNoiseShader& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
    , fMatrix(total_matrix(rec, shader)) // used for temp storage, adjusted below
    , fPaintingData(shader.fTileSize, shader.fSeed, shader.fBaseFrequencyX,
                    shader.fBaseFrequencyY, fMatrix)
{
    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). The same adjustment is in the setData() function.
    fMatrix.setTranslate(-fMatrix.getTranslateX() + SK_Scalar1,
                         -fMatrix.getTranslateY() + SK_Scalar1);
}

void SkPerlinNoiseShader::PerlinNoiseShaderContext::shadeSpan(
        int x, int y, SkPMColor result[], int count) {
    SkPoint point = SkPoint::Make(SkIntToScalar(x), SkIntToScalar(y));
    StitchData stitchData;
    for (int i = 0; i < count; ++i) {
        result[i] = shade(point, stitchData);
        point.fX += SK_Scalar1;
    }
}

#if defined(SK_GRAPHITE)

// If either of these change then the corresponding change must also be made in the SkSL
// perlin_noise_shader function.
static_assert((int)SkPerlinNoiseShader::kFractalNoise_Type ==
              (int)skgpu::graphite::PerlinNoiseShaderBlock::Type::kFractalNoise);
static_assert((int)SkPerlinNoiseShader::kTurbulence_Type ==
              (int)skgpu::graphite::PerlinNoiseShaderBlock::Type::kTurbulence);

// If kBlockSize changes here then it must also be changed in the SkSL noise_function
// implementation.
static_assert(kBlockSize == 256);

void SkPerlinNoiseShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                       skgpu::graphite::PaintParamsKeyBuilder* builder,
                                       skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkASSERT(fNumOctaves);

    SkMatrix totalMatrix = keyContext.local2Dev().asM33();
    if (keyContext.localMatrix()) {
        totalMatrix.preConcat(*keyContext.localMatrix());
    }

    SkMatrix invTotal;
    bool result = totalMatrix.invert(&invTotal);
    if (!result) {
        SKGPU_LOG_W("Couldn't invert totalMatrix for PerlinNoiseShader");

        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    auto paintingData = std::make_unique<SkPerlinNoiseShader::PaintingData>(fTileSize,
                                                                                fSeed,
                                                                                fBaseFrequencyX,
                                                                                fBaseFrequencyY,
                                                                                totalMatrix);

    sk_sp<TextureProxy> perm =
            RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                            paintingData->getPermutationsBitmap());

    sk_sp<TextureProxy> noise = RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                                                paintingData->getNoiseBitmap());

    if (!perm || !noise) {
        SKGPU_LOG_W("Couldn't create tables for PerlinNoiseShader");

        SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
        builder->endBlock();
        return;
    }

    PerlinNoiseShaderBlock::PerlinNoiseData data(static_cast<PerlinNoiseShaderBlock::Type>(fType),
                                                 paintingData->fBaseFrequency,
                                                 fNumOctaves,
                                                 { paintingData->fStitchDataInit.fWidth,
                                                   paintingData->fStitchDataInit.fHeight });

    data.fPermutationsProxy = std::move(perm);
    data.fNoiseProxy = std::move(noise);

    // This (1,1) translation is due to WebKit's 1 based coordinates for the noise
    // (as opposed to 0 based, usually). Remember: this matrix (shader2World) is going to be
    // inverted before being applied.
    SkMatrix shader2Local = SkMatrix::Translate(-1 + totalMatrix.getTranslateX(),
                                                -1 + totalMatrix.getTranslateY());
    shader2Local.postConcat(invTotal);

    LocalMatrixShaderBlock::LMShaderData lmShaderData(shader2Local);

    KeyContextWithLocalMatrix newContext(keyContext, shader2Local);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);
        PerlinNoiseShaderBlock::BeginBlock(newContext, builder, gatherer, &data);
        builder->endBlock();
    builder->endBlock();
}
#endif // SK_GRAPHITE

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_input(SkScalar baseX, SkScalar baseY, int numOctaves, const SkISize* tileSize,
                        SkScalar seed) {
    if (!(baseX >= 0 && baseY >= 0)) {
        return false;
    }
    if (!(numOctaves >= 0 && numOctaves <= SkPerlinNoiseShader::kMaxOctaves)) {
        return false;
    }
    if (tileSize && !(tileSize->width() >= 0 && tileSize->height() >= 0)) {
        return false;
    }
    if (!SkScalarIsFinite(seed)) {
        return false;
    }
    return true;
}

void SkRegisterPerlinNoiseShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkPerlinNoiseShader);
    // Previous name
    SkFlattenable::Register("SkPerlinNoiseShaderImpl", SkPerlinNoiseShader::CreateProc);
}

namespace SkShaders {
sk_sp<SkShader> MakeFractalNoise(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                                 int numOctaves, SkScalar seed, const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kFractalNoise, w/o any octaves, the entire shader collapses to:
        //    [0,0,0,0] * 0.5 + 0.5
        constexpr SkColor4f kTransparentGray = {0.5f, 0.5f, 0.5f, 0.5f};

        return SkShaders::Color(kTransparentGray, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShader::kFractalNoise_Type,
                                                       baseFrequencyX, baseFrequencyY, numOctaves,
                                                       seed, tileSize));
}

sk_sp<SkShader> MakeTurbulence(SkScalar baseFrequencyX, SkScalar baseFrequencyY,
                               int numOctaves, SkScalar seed, const SkISize* tileSize) {
    if (!valid_input(baseFrequencyX, baseFrequencyY, numOctaves, tileSize, seed)) {
        return nullptr;
    }

    if (0 == numOctaves) {
        // For kTurbulence, w/o any octaves, the entire shader collapses to: [0,0,0,0]
        return SkShaders::Color(SkColors::kTransparent, /* colorSpace= */ nullptr);
    }

    return sk_sp<SkShader>(new SkPerlinNoiseShader(SkPerlinNoiseShader::kTurbulence_Type,
                                                       baseFrequencyX, baseFrequencyY, numOctaves,
                                                       seed, tileSize));
}

}  // namespace SkShaders
