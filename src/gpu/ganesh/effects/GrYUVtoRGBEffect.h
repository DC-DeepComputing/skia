/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkString.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVAInfoLocation.h" // IWYU pragma: keep
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSamplerState.h"

#include <memory>

class GrYUVATextureProxies;
enum SkYUVColorSpace : int;
namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;
struct SkRect;

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const GrYUVATextureProxies& yuvaProxies,
                                                     GrSamplerState samplerState,
                                                     const GrCaps&,
                                                     const SkMatrix& localMatrix = SkMatrix::I(),
                                                     const SkRect* subset = nullptr,
                                                     const SkRect* domain = nullptr);
    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "YUVtoRGBEffect"; }

private:
    GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4],
                     int numPlanes,
                     const SkYUVAInfo::YUVALocations&,
                     const bool snap[2],
                     bool packedYUYV,
                     const SkMatrix& packedPlaneMatrix,
                     float packedFirstTexelCenter,
                     float packedLastTexelCenter,
                     SkYUVColorSpace yuvColorSpace);

    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override;
#endif

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkYUVAInfo::YUVALocations   fLocations;
    SkYUVColorSpace             fYUVColorSpace;
    bool                        fSnap[2];
    bool                        fPackedYUYV = false;
    // The packed plane's texel space equals the image space transformed by
    // the encoded origin. The effect applies this inverse-origin matrix to
    // the sample coordinates before computing the texel-parity sampling, and
    // the plane child is created with an identity matrix. Only meaningful
    // when fPackedYUYV is set.
    SkMatrix                    fPackedPlaneMatrix = SkMatrix::I();
    // Center x of the first/last texel of the sampled region of the packed
    // plane, in plane texel coordinates. The second texel sample (the pixel
    // pair's partner texel) is clamped to this range so pixels at the edges
    // of the region duplicate the edge chroma instead of sampling outside
    // the subset. Only meaningful when fPackedYUYV is set.
    float                       fPackedFirstTexelCenter = 0.f;
    float                       fPackedLastTexelCenter = 0.f;
};
#endif
