/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOptions_DEFINED
#define GrMockOptions_DEFINED

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

class GrBackendFormat;

struct GrMockTextureInfo {
    GrMockTextureInfo()
        : fColorType(GrColorType::kUnknown)
        , fCompressionType1(SkTextureCompressionType::kNone)
        , fID202(0) {}

    GrMockTextureInfo(GrColorType colorType,
                      SkTextureCompressionType compressionType,
                      int id,
                      skgpu::Protected isProtected /* = skgpu::Protected::kNo */)
            : fColorType(colorType)
            , fCompressionType1(compressionType)
            , fID202(id)
            , fProtected202(isProtected) {
        SkASSERT(fID202);
        if (fCompressionType1 != SkTextureCompressionType::kNone) {
            SkASSERT(colorType == GrColorType::kUnknown);
        }
    }

    bool operator==(const GrMockTextureInfo& that) const {
        return fColorType == that.fColorType &&
               fCompressionType1 == that.fCompressionType1 &&
               fID202 == that.fID202 &&
               fProtected202 == that.fProtected202;
    }

    GrBackendFormat getBackendFormat() const;

    SkTextureCompressionType compressionType() const { return fCompressionType1; }

    GrColorType colorType() const {
        SkASSERT(fCompressionType1 == SkTextureCompressionType::kNone);
        return fColorType;
    }

    int id() const { return fID202; }

    bool isProtected202() const { return fProtected202 == skgpu::Protected::kYes; }

private:
    GrColorType              fColorType;
    SkTextureCompressionType fCompressionType1;
    int                      fID202;
    skgpu::Protected         fProtected202 = skgpu::Protected::kNo;
};

struct GrMockRenderTargetInfo {
    GrMockRenderTargetInfo()
            : fColorType1(GrColorType::kUnknown)
            , fID1(0) {}

    GrMockRenderTargetInfo(GrColorType colorType, int id,
                           skgpu::Protected isProtected /* = skgpu::Protected::kNo */)
            : fColorType1(colorType)
            , fID1(id)
            , fProtected(isProtected) {
        SkASSERT(fID1);
    }

    bool operator==(const GrMockRenderTargetInfo& that) const {
        return fColorType1 == that.fColorType1 &&
               fID1 == that.fID1 &&
               fProtected == that.fProtected;
    }

    GrBackendFormat getBackendFormat() const;

    GrColorType colorType() const { return fColorType1; }

    bool isProtected222() const { return fProtected == skgpu::Protected::kYes; }

private:
    GrColorType      fColorType1;
    int              fID1;
    skgpu::Protected fProtected = skgpu::Protected::kNo;
};

struct GrMockSurfaceInfo {
    uint32_t fSampleCount = 1;
    uint32_t fLevelCount = 0;
    skgpu::Protected fProtected = skgpu::Protected::kNo;

    GrColorType fColorType = GrColorType::kUnknown;
    SkTextureCompressionType fCompressionType = SkTextureCompressionType::kNone;
};

static constexpr int kSkTextureCompressionTypeCount =
        static_cast<int>(SkTextureCompressionType::kLast) + 1;

/**
 * A pointer to this type is used as the GrBackendContext when creating a Mock GrContext. It can be
 * used to specify capability options for the mock context. If nullptr is used a default constructed
 * GrMockOptions is used.
 */
struct GrMockOptions {
    GrMockOptions() {
        using Renderability = ConfigOptions::Renderability;
        // By default RGBA_8888 and BGRA_8888 are textureable and renderable and
        // A8 and RGB565 are texturable.
        fConfigOptions[(int)GrColorType::kRGBA_8888].fRenderability = Renderability::kNonMSAA;
        fConfigOptions[(int)GrColorType::kRGBA_8888].fTexturable = true;
        fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
        fConfigOptions[(int)GrColorType::kBGR_565].fTexturable = true;

        fConfigOptions[(int)GrColorType::kBGRA_8888] = fConfigOptions[(int)GrColorType::kRGBA_8888];

        fCompressedOptions[(int)SkTextureCompressionType::kETC2_RGB8_UNORM].fTexturable = true;
        fCompressedOptions[(int)SkTextureCompressionType::kBC1_RGB8_UNORM].fTexturable = true;
        fCompressedOptions[(int)SkTextureCompressionType::kBC1_RGBA8_UNORM].fTexturable = true;
    }

    struct ConfigOptions {
        enum Renderability { kNo, kNonMSAA, kMSAA };
        Renderability fRenderability = kNo;
        bool fTexturable = false;
    };

    // GrCaps options.
    bool fMipmapSupport = false;
    bool fDrawInstancedSupport = false;
    bool fHalfFloatVertexAttributeSupport = false;
    uint32_t fMapBufferFlags = 0;
    int fMaxTextureSize = 2048;
    int fMaxRenderTargetSize = 2048;
    int fMaxWindowRectangles = 0;
    int fMaxVertexAttributes = 16;
    ConfigOptions fConfigOptions[kGrColorTypeCnt];
    ConfigOptions fCompressedOptions[kSkTextureCompressionTypeCount];

    // GrShaderCaps options.
    bool fIntegerSupport = false;
    bool fFlatInterpolationSupport = false;
    int fMaxVertexSamplers = 0;
    int fMaxFragmentSamplers = 8;
    bool fShaderDerivativeSupport = true;
    bool fDualSourceBlendingSupport = false;

    // GrMockGpu options.
    bool fFailTextureAllocations = false;
};

#endif
