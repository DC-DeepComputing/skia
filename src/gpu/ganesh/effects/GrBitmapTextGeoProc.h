/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextGeoProc_DEFINED
#define GrBitmapTextGeoProc_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkSize.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSamplerState.h"

#include <memory>

class GrColorSpaceXform;
class GrSurfaceProxyView;
struct GrShaderCaps;

namespace skgpu {
class KeyBuilder;
enum class MaskFormat : int;
}

/**
 * The output color of this effect is a modulation of the input color and a sample from a texture.
 * It allows explicit specification of the filtering and wrap modes (GrSamplerState). The input
 * coords are a custom attribute.
 */
class GrBitmapTextGeoProc : public GrGeometryProcessor {
public:
    inline static constexpr int kMaxTextures = 4;

    static GrGeometryProcessor* Make(SkArenaAlloc*,
                                     const GrShaderCaps&,
                                     const SkPMColor4f&,
                                     bool wideColor,
                                     sk_sp<GrColorSpaceXform>,
                                     const GrSurfaceProxyView* views,
                                     int numActiveViews,
                                     GrSamplerState,
                                     skgpu::MaskFormat,
                                     const SkMatrix& localMatrix,
                                     bool usesW);

    ~GrBitmapTextGeoProc() override {}

    const char* name() const override { return "BitmapText"; }

    void addNewViews(const GrSurfaceProxyView*, int numActiveViews, GrSamplerState);

    void addToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override;

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps& caps) const override;

private:
    class Impl;

    GrBitmapTextGeoProc(const GrShaderCaps&, const SkPMColor4f&, bool wideColor,
                        sk_sp<GrColorSpaceXform> colorSpaceXform,
                        const GrSurfaceProxyView* views, int numViews, GrSamplerState params,
                        skgpu::MaskFormat format, const SkMatrix& localMatrix, bool usesW);

    bool hasVertexColor() const { return fInColor.isInitialized(); }

    const TextureSampler& onTextureSampler(int i) const override { return fTextureSamplers[i]; }

    SkPMColor4f              fColor;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    SkMatrix                 fLocalMatrix;
    bool                     fUsesW;
    SkISize                  fAtlasDimensions;  // dims for all textures used with fTextureSamplers
    TextureSampler           fTextureSamplers[kMaxTextures];
    Attribute                fInPosition;
    Attribute                fInColor;
    Attribute                fInTextureCoords;
    skgpu::MaskFormat        fMaskFormat;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    using INHERITED = GrGeometryProcessor;
};

#endif
