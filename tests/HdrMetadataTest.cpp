/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "tests/Test.h"

DEF_TEST(HdrMetadata_ParseSerialize_ContentLightLevelInformation, r) {
    uint8_t data[] = {
        0x03, 0xE8,
        0x00, 0xFA,
    };
    // Data taken from:
    // https://www.w3.org/TR/png-3/#example-13
    // https://www.w3.org/TR/png-3/#example-14
    uint8_t dataPng[] = {
        0x00, 0x98, 0x96, 0x80,
        0x00, 0x26, 0x25, 0xA0,
    };
    skhdr::ContentLightLevelInformation clliExpected = {
        1000.f, 250.f,
    };
    auto skData = SkData::MakeWithoutCopy(data, sizeof(data));
    auto skDataPng = SkData::MakeWithoutCopy(dataPng, sizeof(dataPng));

    skhdr::ContentLightLevelInformation clli;
    REPORTER_ASSERT(r, clli.parse(skData.get()));
    REPORTER_ASSERT(r, clli == clliExpected);
    REPORTER_ASSERT(r, skData->equals(clli.serialize().get()));

    skhdr::ContentLightLevelInformation clliPng;
    REPORTER_ASSERT(r, clliPng.parsePngChunk(skDataPng.get()));
    REPORTER_ASSERT(r, clliPng == clliExpected);
    REPORTER_ASSERT(r, skDataPng->equals(clli.serializePngChunk().get()));
}

DEF_TEST(HdrMetadata_ParseSerialize_MasteringDisplayColorVolume, r) {
    // Data taken from:
    // https://www.w3.org/TR/png-3/#example-5
    // https://www.w3.org/TR/png-3/#example-6
    // https://www.w3.org/TR/png-3/#example-7
    // https://www.w3.org/TR/png-3/#example-8
    uint8_t data[] = {
        0x8A, 0x48, 0x39, 0x08, // Red
        0x21, 0x34, 0x9B, 0xAA, // Green
        0x19, 0x96, 0x08, 0xFC, // Blue
        0x3D, 0x13, 0x40, 0x42, // White
        0x02, 0x62, 0x5A, 0x00, // Maximum luminance
        0x00, 0x00, 0x00, 0x05, // Minimum luminance
    };
    skhdr::MasteringDisplayColorVolume mdcvExpected = {
        {0.708f, 0.292f, 0.17f, 0.797f, 0.131f, 0.046f, 0.3127f, 0.329f},
        4000.f, 0.0005f,
    };
    auto skData = SkData::MakeWithoutCopy(data, sizeof(data));

    skhdr::MasteringDisplayColorVolume mdcv;
    REPORTER_ASSERT(r, mdcv.parse(skData.get()));
    REPORTER_ASSERT(r, mdcv == mdcvExpected);
    REPORTER_ASSERT(r, skData->equals(mdcv.serialize().get()));
}

DEF_TEST(HdrMetadata_Agtm_Cubic, r) {
    skhdr::Agtm::PiecewiseCubicFunction cubic = {
        10,
        {0.10720647f, 0.76246667f, 1.39535723f, 2.17572099f, 2.47834070f,
         3.14288223f, 3.35428070f, 4.24864607f, 4.59087493f, 4.80373641f},
        {0.37384606f, 0.93143060f, 0.f,         1.23009354f, 1.25542898f,
         2.22460677f, 2.69226748f, 3.45838813f, 4.44597502f, 5.19196203f},
        {0.},
    };
    cubic.populateSlopeFromPCHIP();

    const float mExpected[10] = { 2.03242568f, 0.f,         0.f,         0.14042951f, 0.14250506f,
                                  1.82245618f, 1.35855757f, 1.43703564f, 3.18918733f, 3.74186390f};
    for (size_t i = 0; i < 10; ++i) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic.fM[i], mExpected[i], 0.0001f));
    }

    const float yExpected[11] = {
        0.37384606f, 0.86280187f, 0.63630745f, 0.05871820f, 1.05625216f,
        1.26009455f, 1.95243885f, 2.85680727f, 3.19521825f, 4.14318213f,
        5.13419092f};
    for (size_t i = 0; i < 11; ++i) {
        const float x = i / 2.f;
        const float y = cubic.evaluate(x);
        REPORTER_ASSERT(r, SkScalarNearlyEqual(y, yExpected[i], 0.0001f));
    }
}

DEF_TEST(HdrMetadata_Agtm_Mix, r) {
    skhdr::Agtm::ComponentMixingFunction mix;
    SkColor4f c;

    mix = skhdr::Agtm::ComponentMixingFunction();
    mix.fMax = 1.f;

    c = SkColor4f({0.75f, 0.5f, 0.25f, 0.125f});
    c = mix.evaluate(c);
    REPORTER_ASSERT(r, c.fR == 0.75);
    REPORTER_ASSERT(r, c.fG == 0.75);
    REPORTER_ASSERT(r, c.fB == 0.75);
    REPORTER_ASSERT(r, c.fA == 0.125);

    mix = skhdr::Agtm::ComponentMixingFunction();
    mix.fMax = 0.25f;
    mix.fMin = 0.75f;

    c = SkColor4f({1.f, 0.5f, 0.25f, 0.125f});
    c = mix.evaluate(c);
    REPORTER_ASSERT(r, c.fR == 0.4375);
    REPORTER_ASSERT(r, c.fG == 0.4375);
    REPORTER_ASSERT(r, c.fB == 0.4375);
    REPORTER_ASSERT(r, c.fA == 0.125);

    mix = skhdr::Agtm::ComponentMixingFunction();
    mix.fMax = 0.125f;
    mix.fMin = 0.25f;
    mix.fComponent = 0.625f;

    c = SkColor4f({1.f, 0.5f, 0.25f, 1.f});
    c = mix.evaluate(c);
    REPORTER_ASSERT(r, c.fR == 0.8125f);
    REPORTER_ASSERT(r, c.fG == 0.5f);
    REPORTER_ASSERT(r, c.fB == 0.34375f);
    REPORTER_ASSERT(r, c.fA == 1.f);
}

DEF_TEST(HdrMetadata_Agtm_RWTMO, r) {
    skhdr::Agtm agtm;
    agtm.fBaselineHdrHeadroom = 1.f;
    agtm.populateUsingRwtmo();

    REPORTER_ASSERT(r, memcmp(&agtm.fGainApplicationSpacePrimaries, &SkNamedPrimaries::kRec2020,
                              sizeof(SkColorSpacePrimaries)) == 0);
    REPORTER_ASSERT(r, agtm.fNumAlternateImages == 2);
    REPORTER_ASSERT(r, agtm.fAlternateHdrHeadroom[0] == 0.f);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(agtm.fAlternateHdrHeadroom[1], 0.6151137835929048f));

    const float xExpected[2][8] = {
        {1.00000f, 1.06461f, 1.15531f, 1.27209f, 1.41494f, 1.58388f, 1.77890f, 2.00000f},
        {1.00000f, 1.10504f, 1.22269f, 1.35294f, 1.49580f, 1.65126f, 1.81933f, 2.00000f},
    };
    const float yExpected[2][8] = {
        {-0.35356f, -0.37367f, -0.42913f, -0.51246f, -0.61663f, -0.73563f, -0.86465f, -1.00000f},
        { 0.00000f, -0.01253f, -0.04583f, -0.09477f, -0.15559f, -0.22550f, -0.30244f, -0.38489f},
    };
    const float mExpected[2][8] = {
        {0.00000f, -0.50266f, -0.68079f, -0.73059f, -0.72159f, -0.68535f, -0.63784f, -0.58742f},
        {0.00000f, -0.21470f, -0.33759f, -0.40581f, -0.44088f, -0.45573f, -0.45828f, -0.45351f},
    };

    for (size_t a = 0; a < 2; ++a) {
        const auto& cubic = agtm.fGainFunction[a].fPiecewiseCubic;
        REPORTER_ASSERT(r, cubic.fNumControlPoints == 8);
        for (size_t c = 0; c < 8; ++c) {
            REPORTER_ASSERT(r, SkScalarNearlyEqual(xExpected[a][c], cubic.fX[c]));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(yExpected[a][c], cubic.fY[c]));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(mExpected[a][c], cubic.fM[c]));
        }
    }
}

DEF_TEST(HdrMetadata_Agtm_Weighting, r) {
    skhdr::Agtm agtm;
    skhdr::Agtm::Weighting w;

    agtm.fBaselineHdrHeadroom = 1.f;
    {
        float H[] = {0.f, 1.f, 2.f};
        for (auto h : H) {
            w = agtm.ComputeWeighting(h);
            REPORTER_ASSERT(r, w.fWeight[0] == 0.f);
            REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
            REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == skhdr::Agtm::Weighting::kInvalidIndex);
            REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);
        }
    }

    {
        agtm.fBaselineHdrHeadroom = 1.f;
        agtm.fNumAlternateImages = 1;
        agtm.fAlternateHdrHeadroom[0] = 0.f;

        w = agtm.ComputeWeighting(0.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 1.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 0);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(0.25f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.75f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 0);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(1.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == skhdr::Agtm::Weighting::kInvalidIndex);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(1.25f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == skhdr::Agtm::Weighting::kInvalidIndex);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);
    }

    {
        agtm.fBaselineHdrHeadroom = 1.f;
        agtm.fNumAlternateImages = 2;
        agtm.fAlternateHdrHeadroom[0] = 0.f;
        agtm.fAlternateHdrHeadroom[1] = 2.f;

        w = agtm.ComputeWeighting(0.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 1.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 0);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(0.25f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.75f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 0);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(1.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == skhdr::Agtm::Weighting::kInvalidIndex);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(1.25f);
        REPORTER_ASSERT(r, w.fWeight[0] == 0.25f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 1);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(2.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 1.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 1);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);

        w = agtm.ComputeWeighting(3.f);
        REPORTER_ASSERT(r, w.fWeight[0] == 1.f);
        REPORTER_ASSERT(r, w.fWeight[1] == 0.f);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == 1);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == skhdr::Agtm::Weighting::kInvalidIndex);
    }
}

