/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

#include "modules/skcms/skcms.h"
#include "tests/Test.h"

// TODO / DO NOT SUBMIT: Ask Brian how to run clang-format

static void InitSrgbProfile(skcms_ICCProfile* out) {
    skcms_Matrix3x3 toXYZD50 = skcms_sRGB_profile()->toXYZD50;
    skcms_Init(out);
    skcms_SetTransferFunction(out, skcms_sRGB_TransferFunction());
    skcms_SetXYZD50(out, &toXYZD50);
}

static void TestRoundtripOfPixelFormat(skiatest::Reporter* reporter,
                                       skcms_PixelFormat pixelFormat1,
                                       skcms_PixelFormat pixelFormat2,
                                       const std::vector<uint8_t>& expectedData1,
                                       const std::vector<uint8_t>& expectedData2,
                                       size_t nPixels) {
    // Don't transform the alpha format nor color profile.
    const skcms_AlphaFormat kAlphaFormat = skcms_AlphaFormat_Unpremul;
    skcms_ICCProfile kProfile;
    InitSrgbProfile(&kProfile);

    // Transform `expectedData1` into `actualData2` and compare with `expectedData2`.
    std::vector<uint8_t> actualData2(expectedData2.size(), 123);
    bool success = skcms_Transform(expectedData1.data(),
                                   pixelFormat1,
                                   kAlphaFormat,
                                   &kProfile,
                                   actualData2.data(),
                                   pixelFormat2,
                                   kAlphaFormat,
                                   &kProfile,
                                   nPixels);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, actualData2 == expectedData2);

    // Transform `expectedData2` into `actualData1` and compare with `expectedData1`.
    std::vector<uint8_t> actualData1(expectedData1.size(), 123);
    success = skcms_Transform(expectedData2.data(),
                              pixelFormat2,
                              kAlphaFormat,
                              &kProfile,
                              actualData1.data(),
                              pixelFormat1,
                              kAlphaFormat,
                              &kProfile,
                              nPixels);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, actualData1 == expectedData1);
}

DEF_TEST(PixelFormatRoundTripOfG8AndRgba8888, reporter) {
    std::vector<uint8_t> data1{0, 1, 2, 3, 4};
    std::vector<uint8_t> data2{0, 0,    0, 0xff, 1, 1,    1, 0xff, 2, 2,
                               2, 0xff, 3, 3,    3, 0xff, 4, 4,    4, 0xff};
    TestRoundtripOfPixelFormat(
            reporter, skcms_PixelFormat_G_8, skcms_PixelFormat_RGBA_8888, data1, data2, 5);
}

DEF_TEST(PixelFormatRoundTripOfGA88AndRgba8888, reporter) {
    std::vector<uint8_t> data1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<uint8_t> data2{0, 0, 0, 1, 2, 2, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7, 8, 8, 8, 9};
    TestRoundtripOfPixelFormat(
            reporter, skcms_PixelFormat_GA_88, skcms_PixelFormat_RGBA_8888, data1, data2, 5);
}

DEF_TEST(PixelFormatRoundTripOfRgb888AndRgba8888, reporter) {
    std::vector<uint8_t> data1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    std::vector<uint8_t> data2{0, 1,    2, 0xFF, 3,  4,    5,  0xFF, 6,  7,
                               8, 0xFF, 9, 10,   11, 0xFF, 12, 13,   14, 0xFF};
    TestRoundtripOfPixelFormat(
            reporter, skcms_PixelFormat_RGB_888, skcms_PixelFormat_RGBA_8888, data1, data2, 5);
}
