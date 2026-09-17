/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkYUVAInfo.h"
#include "tests/Test.h"

using PlaneConfig = SkYUVAInfo::PlaneConfig;
using Subsampling = SkYUVAInfo::Subsampling;

DEF_TEST(SkYUVAInfoPackedYUYV, reporter) {
    // The packed 4:2:2 config is only compatible with horizontal 2:1
    // subsampling.
    for (Subsampling s : {Subsampling::kUnknown, Subsampling::k444, Subsampling::k422,
                          Subsampling::k420, Subsampling::k440, Subsampling::k411,
                          Subsampling::k410}) {
        SkYUVAInfo info({1920, 1080}, PlaneConfig::kYUYV, s, kRec601_SkYUVColorSpace);
        REPORTER_ASSERT(reporter, info.isValid() == (s == Subsampling::k422));
    }

    SkYUVAInfo info({1920, 1080}, PlaneConfig::kYUYV, Subsampling::k422,
                    kRec601_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, info.isValid());
    REPORTER_ASSERT(reporter, info.numPlanes() == 1);
    REPORTER_ASSERT(reporter, !info.hasAlpha());
    REPORTER_ASSERT(reporter, info.numChannelsInPlane(0) == 2);
    REPORTER_ASSERT(reporter, info.numChannelsInPlane(1) == 0);

    // The packed plane is stored at full image texel dimensions even though it
    // is chroma-subsampled (each texel covers two pixels).
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    REPORTER_ASSERT(reporter, info.planeDimensions(planeDimensions) == 1);
    REPORTER_ASSERT(reporter, planeDimensions[0] == SkISize{1920, 1080});

    // The plane is sampled at full rate; the packed extraction happens in the
    // YUV effect.
    auto [ssx, ssy] = info.planeSubsamplingFactors(0);
    REPORTER_ASSERT(reporter, ssx == 1 && ssy == 1);

    // Odd image widths are not representable (the last pixel would be
    // unpaired) and are rejected by the constructor.
    SkYUVAInfo odd({1919, 1080}, PlaneConfig::kYUYV, Subsampling::k422,
                   kRec601_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, !odd.isValid());

    // Channel locations: Y is channel 0 (R) of the plane, U and V are both
    // channel 1 (G) of adjacent texels; there is no alpha.
    const uint32_t rgFlags = kRG_SkColorChannelFlags;
    auto locations = info.toYUVALocations(&rgFlags);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fChannel ==
                                 SkColorChannel::kR);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fChannel ==
                                 SkColorChannel::kG);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fChannel ==
                                 SkColorChannel::kG);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kA].fPlane < 0);
}

DEF_TEST(SkYUVAInfoSinglePlaneConfigs, reporter) {
    // Y410 is modeled as kUYVA with k444 subsampling: plane channel order is
    // U, Y, V, A in R, G, B, A.
    SkYUVAInfo y410({1920, 1080}, PlaneConfig::kUYVA, Subsampling::k444,
                    kRec709_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, y410.isValid());
    REPORTER_ASSERT(reporter, y410.numPlanes() == 1);
    REPORTER_ASSERT(reporter, y410.hasAlpha());
    REPORTER_ASSERT(reporter, y410.numChannelsInPlane(0) == 4);

    const uint32_t rgbaFlags = kRGBA_SkColorChannelFlags;
    auto locations = y410.toYUVALocations(&rgbaFlags);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fChannel ==
                                 SkColorChannel::kG);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fChannel ==
                                 SkColorChannel::kR);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fChannel ==
                                 SkColorChannel::kB);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kA].fChannel ==
                                 SkColorChannel::kA);
}
