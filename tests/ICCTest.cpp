/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkICC.h"
#include "modules/skcms/skcms.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> profile = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));
        REPORTER_ASSERT(r, !parsed.has_CICP);

        auto got  = SkColorSpace::Make(parsed);
        auto want = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), want.get()));
    }
}

DEF_TEST(HDR_ICC, r) {
    constexpr size_t kTestCount = 3;
    std::array<sk_sp<SkData>, kTestCount> profile = {
            SkWriteICCProfile(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020),
            SkWriteICCProfile(SkNamedTransferFn::kHLG, SkNamedGamut::kDisplayP3),
            SkWriteICCProfile(SkNamedTransferFn::kSRGB, SkNamedGamut::kSRGB),
    };

    std::array<sk_sp<SkData>, kTestCount> dst_profile = {
            SkWriteICCProfile(SkNamedTransferFn::kLinear, SkNamedGamut::kRec2020),
            SkWriteICCProfile(SkNamedTransferFn::kLinear, SkNamedGamut::kDisplayP3),
            SkWriteICCProfile(SkNamedTransferFn::kLinear, SkNamedGamut::kSRGB),
    };

    constexpr size_t kPixelCount = 7;

    // clang-format off
    float pixels[kPixelCount][3] = {
            { 0.00f, 0.00f, 0.00f, },
            { 0.50f, 0.50f, 0.50f, },
            { 0.50f, 0.00f, 0.00f, },
            { 0.00f, 0.50f, 0.00f, },
            { 0.00f, 0.00f, 0.50f, },
            { 0.25f, 0.50f, 0.00f, },
            { 0.75f, 0.75f, 0.75f, },
    };

    // The tone mapped value of PQ 0.5 and 0.75.
    constexpr float kPQ_05 = 0.3182877451f;
    constexpr float kPQ_075 = 0.9943588777f;

    // The tone mapped value of PQ 0.25, when maxRGB is 0.5.
    constexpr float kPQ_025 = 0.020679904f;

    // The tone mapped value of HLG 0.5 and 0.75 (when all channels are equal).
    constexpr float kHLG_05 = 0.20188954163f;
    constexpr float kHLG_075 = 0.5208149688f;

    // The linearized values of sRGB 0.25, 0.5, and 0.75.
    constexpr float kSRGB_025 = 0.05087607f;
    constexpr float kSRGB_05  = 0.21404112f;
    constexpr float kSRGB_075 = 0.52252153f;

    float dst_pixels_expected[kTestCount][kPixelCount][3] = {
            {
                    { 0.f,     0.f,     0.f,     },
                    { kPQ_05,  kPQ_05,  kPQ_05,  },
                    { kPQ_05,  0.f,     0.f,     },
                    { 0.f,     kPQ_05,  0.f,     },
                    { 0.f,     0.f,     kPQ_05,  },
                    { kPQ_025, kPQ_05,  0.f,     },
                    { kPQ_075, kPQ_075, kPQ_075, }, // PQ maps 0.75 ~ 1000 nits to 1.0
            },
            {
                    { 0.f,      0.f,      0.f,      },
                    { kHLG_05,  kHLG_05,  kHLG_05,  },
                    { 0.1618f,  0.f,      0.f,      }, // HLG will map 0.5 to different values
                    { 0.f,      0.1895f,  0.f,      }, // if it is the R, G, or B channel, because
                    { 0.f,      0.f,      0.1251f,  }, // of the OOTF.
                    { 0.0513f,  0.1924f,  0.f,      },
                    { kHLG_075, kHLG_075, kHLG_075, },
            },
            {
                    { 0.f,       0.f,       0.f,       },
                    { kSRGB_05,  kSRGB_05,  kSRGB_05,  }, // This is just the sRGB transfer function
                    { kSRGB_05,  0.f,       0.f,       },
                    { 0.f,       kSRGB_05,  0.f,       },
                    { 0.f,       0.f,       kSRGB_05,  },
                    { kSRGB_025, kSRGB_05,  0.f,       },
                    { kSRGB_075, kSRGB_075, kSRGB_075, },
            },
    };
    // clang-format on
    std::array<bool, kTestCount> cicp_expected = {true, true, false};
    std::array<bool, kTestCount> a2b_expected = {true, true, false};
    std::array<uint32_t, kTestCount> cicp_primaries_expected = {9, 12, 0};
    std::array<uint32_t, kTestCount> cicp_trfn_expected = {16, 18, 0};

    for (size_t test = 0; test < kTestCount; ++test) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile[test]->data(), profile[test]->size(), &parsed));

        REPORTER_ASSERT(r, parsed.has_A2B == a2b_expected[test]);
        REPORTER_ASSERT(r, parsed.has_CICP == cicp_expected[test]);
        if (cicp_expected[test]) {
            REPORTER_ASSERT(r, parsed.CICP.color_primaries == cicp_primaries_expected[test]);
            REPORTER_ASSERT(r, parsed.CICP.transfer_characteristics == cicp_trfn_expected[test]);
            REPORTER_ASSERT(r, parsed.CICP.matrix_coefficients == 0);
            REPORTER_ASSERT(r, parsed.CICP.video_full_range_flag == 1);
        }
        // Disable the CICP tag so we can test the A2B transform (otherwise the CICP takes
        // precedence).
        parsed.has_CICP = false;

        skcms_ICCProfile dst_parsed;
        REPORTER_ASSERT(
                r, skcms_Parse(dst_profile[test]->data(), dst_profile[test]->size(), &dst_parsed));

        for (size_t pixel = 0; pixel < kPixelCount; ++pixel) {
            std::array<float, 3> dst_pixel_actual = {0.f, 0.f, 0.f};
            bool xform_result = skcms_Transform(pixels[pixel],
                                                skcms_PixelFormat_RGB_fff,
                                                skcms_AlphaFormat_Opaque,
                                                &parsed,
                                                dst_pixel_actual.data(),
                                                skcms_PixelFormat_RGB_fff,
                                                skcms_AlphaFormat_Opaque,
                                                &dst_parsed,
                                                1);
            REPORTER_ASSERT(r, xform_result);

            auto approx_equal = [=](float x, float y) { return std::abs(x - y) < 1.f / 64.f; };
            for (size_t i = 0; i < 3; ++i) {
                REPORTER_ASSERT(
                        r, approx_equal(dst_pixel_actual[i], dst_pixels_expected[test][pixel][i]));
            }
        }
    }
}

DEF_TEST(ICC_WriteRejectsInvalidColorSpace, r) {
    REPORTER_ASSERT(r, SkWriteICCProfile(static_cast<const SkColorSpace*>(nullptr)) == nullptr);
    REPORTER_ASSERT(
            r, SkWriteICCProfile(static_cast<const skcms_ICCProfile*>(nullptr), "desc") == nullptr);

    // Non-invertible, NaN/Inf, or s15Fixed16-underflowing/saturating toXYZD50 matrices must be
    // rejected instead of emitting an ICC profile with zero or singular rXYZ/gXYZ/bXYZ tags.
    constexpr float kNaN = std::numeric_limits<float>::quiet_NaN();
    constexpr float kInf = std::numeric_limits<float>::infinity();
    constexpr skcms_Matrix3x3 kBadMatrices[] = {
            {{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
            {{{1, 0, 0}, {1, 0, 0}, {0, 0, 1}}},
            {{{kNaN, 0, 0}, {0, 1, 0}, {0, 0, 1}}},
            {{{kInf, 0, 0}, {0, 1, 0}, {0, 0, 1}}},
            {{{1e-7f, 0, 0}, {0, 1e-7f, 0}, {0, 0, 1e-7f}}},
            {{{1e8f, 2e8f, 0}, {1e8f, 1e8f, 0}, {0, 0, 1}}},
            {{{1e35f, 2e35f, 0}, {1e35f, 1e35f, 0}, {0, 0, 1}}},
    };
    for (const skcms_Matrix3x3& badMatrix : kBadMatrices) {
        REPORTER_ASSERT(r, SkWriteICCProfile(SkNamedTransferFn::kSRGB, badMatrix) == nullptr);
        REPORTER_ASSERT(r, SkWriteICCProfile(SkNamedTransferFn::kPQ, badMatrix) == nullptr);
        sk_sp<SkColorSpace> cs = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, badMatrix);
        REPORTER_ASSERT(r, cs);
        REPORTER_ASSERT(r, SkWriteICCProfile(cs.get()) == nullptr);

        skcms_ICCProfile profile;
        skcms_Init(&profile);
        skcms_SetTransferFunction(&profile, &SkNamedTransferFn::kSRGB);
        skcms_SetXYZD50(&profile, &badMatrix);
        REPORTER_ASSERT(r, SkWriteICCProfile(&profile, "bad matrix") == nullptr);
    }

    // Non-invertible or unsupported transfer functions must also be rejected.
    skcms_TransferFunction hlgish;
    REPORTER_ASSERT(r,
                    skcms_TransferFunction_makeHLGish(
                            &hlgish, 2.0f, 2.0f, 1.0f / 0.17883277f, 0.28466892f, 0.55991073f));
    skcms_TransferFunction hlgInvish;
    REPORTER_ASSERT(r, skcms_TransferFunction_invert(&hlgish, &hlgInvish));

    const skcms_TransferFunction kBadTrfns[] = {
            {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},    // sRGBish with g = 0
            {1e-7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},   // underflows s15Fixed16 to g = 0
            {1.0f, 1e-7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},   // underflows s15Fixed16 to a = 0
            {2.2f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.0f},    // discontinuous sRGBish
            {-99.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // Invalid
            {-2.0f, kNaN, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},   // PQish with NaN coeff
            hlgInvish,                                     // HLGinvish (no TRC or A2B writer)
    };
    for (const skcms_TransferFunction& badTrfn : kBadTrfns) {
        REPORTER_ASSERT(r, SkWriteICCProfile(badTrfn, SkNamedGamut::kSRGB) == nullptr);

        skcms_ICCProfile profile;
        skcms_Init(&profile);
        skcms_SetTransferFunction(&profile, &badTrfn);
        skcms_SetXYZD50(&profile, &SkNamedGamut::kSRGB);
        REPORTER_ASSERT(r, SkWriteICCProfile(&profile, "bad trfn") == nullptr);
    }

    // An empty skcms_ICCProfile, non-RGB data_color_space, 1-entry curv, or raw toProfile() for PQ
    // (which puts PQ into trc[] without A2B) must return nullptr cleanly without asserting.
    skcms_ICCProfile empty;
    skcms_Init(&empty);
    REPORTER_ASSERT(r, SkWriteICCProfile(&empty, "empty") == nullptr);

    skcms_ICCProfile grayProfile;
    skcms_Init(&grayProfile);
    grayProfile.data_color_space = skcms_Signature_Gray;
    skcms_SetTransferFunction(&grayProfile, &SkNamedTransferFn::kSRGB);
    skcms_SetXYZD50(&grayProfile, &SkNamedGamut::kSRGB);
    REPORTER_ASSERT(r, SkWriteICCProfile(&grayProfile, "gray") == nullptr);

    uint8_t zeroEntry[2] = {0, 0};
    skcms_ICCProfile oneEntryCurv = *skcms_sRGB_profile();
    oneEntryCurv.trc[0].table_entries = 1;
    oneEntryCurv.trc[0].table_16 = zeroEntry;
    REPORTER_ASSERT(r, SkWriteICCProfile(&oneEntryCurv, "curv1") == nullptr);

    skcms_ICCProfile pqToProfile;
    SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020)->toProfile(&pqToProfile);
    REPORTER_ASSERT(r, SkWriteICCProfile(&pqToProfile, "pq without A2B") == nullptr);

    // Malformed A2B/B2A structures (wrong channel counts, grid_points < 2, or non-invertible
    // B2A output_curves) must be rejected, while a valid 3->3 B2A round-trips output_curves.
    constexpr uint16_t kClut[2 * 2 * 2 * 3] = {};
    skcms_ICCProfile lutProfile;
    skcms_Init(&lutProfile);
    lutProfile.has_A2B = true;
    lutProfile.A2B.input_channels = 3;
    lutProfile.A2B.output_channels = 3;
    for (int i = 0; i < 3; ++i) {
        lutProfile.A2B.grid_points[i] = 2;
        lutProfile.A2B.input_curves[i].parametric = SkNamedTransferFn::kLinear;
        lutProfile.A2B.output_curves[i].parametric = SkNamedTransferFn::kLinear;
    }
    lutProfile.A2B.grid_16 = reinterpret_cast<const uint8_t*>(kClut);

    skcms_ICCProfile badGrid = lutProfile;
    badGrid.A2B.grid_points[0] = 1;
    REPORTER_ASSERT(r, SkWriteICCProfile(&badGrid, "bad grid") == nullptr);

    skcms_ICCProfile badChans = lutProfile;
    badChans.A2B.input_channels = 4;
    REPORTER_ASSERT(r, SkWriteICCProfile(&badChans, "bad chans") == nullptr);

    lutProfile.has_B2A = true;
    lutProfile.B2A.input_channels = 3;
    lutProfile.B2A.output_channels = 3;
    for (int i = 0; i < 3; ++i) {
        lutProfile.B2A.grid_points[i] = 2;
        lutProfile.B2A.input_curves[i].parametric = SkNamedTransferFn::kLinear;
        lutProfile.B2A.output_curves[i].parametric = {0.0f, 1.0f, 0, 0, 0, 0, 0};
    }
    lutProfile.B2A.grid_16 = reinterpret_cast<const uint8_t*>(kClut);
    REPORTER_ASSERT(r, SkWriteICCProfile(&lutProfile, "bad b2a output_curves") == nullptr);

    for (int i = 0; i < 3; ++i) {
        lutProfile.B2A.output_curves[i].parametric = SkNamedTransferFn::k2Dot2;
    }
    sk_sp<SkData> b2aData = SkWriteICCProfile(&lutProfile, "valid b2a");
    REPORTER_ASSERT(r, b2aData != nullptr);
    if (b2aData) {
        skcms_ICCProfile parsedB2A;
        REPORTER_ASSERT(r, skcms_Parse(b2aData->data(), b2aData->size(), &parsedB2A));
        REPORTER_ASSERT(r, parsedB2A.has_B2A && parsedB2A.B2A.output_channels == 3);
        REPORTER_ASSERT(r, std::abs(parsedB2A.B2A.output_curves[0].parametric.g - 2.2f) < 1e-3f);
    }
}

DEF_TEST(ICC_WriteDesaturatedOutOfLocusGamut, r) {
    // A desaturating display profile replaces M_true with M_true * D^-1 (mixing channels toward
    // luminance). Its primaries lie outside the [0, 1] chromaticity box (so PrimariesToXYZD50
    // rejects those chromaticities), yet its 3x3 toXYZD50 matrix is finite and invertible and
    // must serialize and round-trip cleanly for both SDR and HDR transfer functions.
    for (float s : {0.45f, 0.60f}) {
        const float wR = 0.2225f * (1.0f - s);
        const float wG = 0.7169f * (1.0f - s);
        const float wB = 0.0606f * (1.0f - s);
        const skcms_Matrix3x3 desat = {{
                {s + wR, wG, wB},
                {wR, s + wG, wB},
                {wR, wG, s + wB},
        }};
        skcms_Matrix3x3 invDesat;
        REPORTER_ASSERT(r, skcms_Matrix3x3_invert(&desat, &invDesat));
        const skcms_Matrix3x3 gamut = skcms_Matrix3x3_concat(&SkNamedGamut::kDisplayP3, &invDesat);

        for (const skcms_TransferFunction& tf :
             {SkNamedTransferFn::kSRGB, SkNamedTransferFn::kPQ}) {
            sk_sp<SkData> icc = SkWriteICCProfile(tf, gamut);
            REPORTER_ASSERT(r, icc != nullptr);
            if (!icc) {
                continue;
            }
            skcms_ICCProfile parsed;
            REPORTER_ASSERT(r, skcms_Parse(icc->data(), icc->size(), &parsed));
            if (skcms_TransferFunction_isPQ(&tf)) {
                REPORTER_ASSERT(r, parsed.has_CICP && parsed.CICP.color_primaries == 2);
            }
            sk_sp<SkColorSpace> roundTrip = SkColorSpace::Make(parsed);
            REPORTER_ASSERT(r, roundTrip != nullptr);

            constexpr float kSrcPixel[3] = {1.0f, 0.0f, 0.0f};
            float dstPixel[3] = {0.0f, 0.0f, 0.0f};
            REPORTER_ASSERT(r,
                            skcms_Transform(kSrcPixel,
                                            skcms_PixelFormat_RGB_fff,
                                            skcms_AlphaFormat_Opaque,
                                            skcms_sRGB_profile(),
                                            dstPixel,
                                            skcms_PixelFormat_RGB_fff,
                                            skcms_AlphaFormat_Opaque,
                                            &parsed,
                                            1));
            REPORTER_ASSERT(r, dstPixel[0] > 0.0f || dstPixel[1] > 0.0f || dstPixel[2] > 0.0f);
        }
    }
}
