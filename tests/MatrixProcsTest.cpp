#include "tests/Test.h"

#include "include/core/SkTypes.h"
#include "include/private/SkFixed.h"
#include "src/core/SkBitmapProcState.h"
#include "src/opts/SkBitmapProcState_opts.h"

int highBits(uint32_t rv) {
    const uint32_t HIGH_14_BITS = ((1 << 14) - 1) << 18;
    return (rv & HIGH_14_BITS) >> 18;
}

int middleBits(uint32_t rv) {
    const uint32_t MIDDLE_4_BITS = ((1 << 4) - 1) << 14;
    return (rv & MIDDLE_4_BITS) >> 14;
}

int lowBits(uint32_t rv) {
    const uint32_t LOW_14_BITS = ((1 << 14) - 1);
    return rv & LOW_14_BITS;
}

struct PackTestCase {
    std::string name;
    SkFixed input;
    unsigned max;
    uint32_t expectedOutput;
};

DEF_TEST(MatrixProcs_pack_clamp, r) {
    // The input values are somewhat arbitrary, inspired by real-world values
    // with some edge cases added as well.
    PackTestCase tests[] = {
        // Negative values keep the fractional part out of convenience, but it is effectively
        // ignored later.
        {"-2.100 => {0x00, 0xe, 0x00}", SkFloatToFixed(-2.100), 63, 0x38000},
        {"-1.900 => {0x00, 0x1, 0x00}", SkFloatToFixed(-1.900), 63, 0x04000},
        {"-0.500 => {0x00, 0x8, 0x00}", SkFloatToFixed(-0.500), 63, 0x20000},
        {"0.0000 => {0x00, 0x0, 0x00}", SkFloatToFixed(0.0000), 63, 0x000001},
        {"0.0416 => {0x00, 0x0, 0x01}", SkFloatToFixed(0.0416), 63, 0x000001},
        {"1.8583 => {0x01, 0xd, 0x02}", SkFloatToFixed(1.8583), 63, 0x074002},
        {"3.6749 => {0x03, 0xa, 0x04}", SkFloatToFixed(3.6749), 63, 0x0e8004},
        {"5.4916 => {0x05, 0x7, 0x06}", SkFloatToFixed(5.4916), 63, 0x15c006},
        {"7.3083 => {0x07, 0x4, 0x08}", SkFloatToFixed(7.3083), 63, 0x1d0008},
        {"9.0000 => {0x09, 0x0, 0x0a}", SkFloatToFixed(9.0000), 63, 0x24000a},
        {"50.000 => {0x32, 0x0, 0x33}", SkFloatToFixed(50.000), 63, 0xc80033},
        {"50.875 => {0x32, 0xe, 0x33}", SkFloatToFixed(50.875), 63, 0xcb8033},
        {"62.123 => {0x3e, 0x1, 0x3f}", SkFloatToFixed(62.123), 63, 0xf8403f},
        {"62.999 => {0x3e, 0xf, 0x3f}", SkFloatToFixed(62.999), 63, 0xfbc03f},
        {"63.000 => {0x3f, 0x0, 0x3f}", SkFloatToFixed(63.000), 63, 0xfc003f},
        // Similarly, overflow keeps the fractional part.
        {"64.500 => {0x3f, 0x8, 0x3f}", SkFloatToFixed(64.500), 63, 0xfe003f},
        {"127.20 => {0x3f, 0x3, 0x3f}", SkFloatToFixed(127.20), 63, 0xfcc03f},
        // Maximum has changed from 63 to 256
        {"64.510 => {0x40, 0x8, 0x41}", SkFloatToFixed(64.510), 256, 0x1020041},
        {"127.21 => {0x7f, 0x3, 0x80}", SkFloatToFixed(127.21), 256, 0x1fcc080},
        {"256.77 => {0x100, 0xc, 0x100}", SkFloatToFixed(256.77), 256, 0x4030100},
        {"10000. => {0x100, 0x0, 0x100}", SkFloatToFixed(10000.), 256, 0x4000100},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(PackTestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        PackTestCase tc = tests[i];
        uint32_t rv = sktests::pack_clamp(tc.input, tc.max);
        uint32_t exp = tc.expectedOutput;

        REPORTER_ASSERT(r, rv == tc.expectedOutput,
                        "%s | %x != %x | {%x, %x, %x} != {%x, %x, %x}\n",
                        tc.name.c_str(), rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }
}

struct UnpackTestCase {
    std::string name;
    uint32_t input;
    int expectedLowerBound;
    int expectedLerp;
    int expectedUpperBound;
};

DEF_TEST(MatrixProcs_unpack_int, r) {
    // These are selected from earlier tests to make sure the packed values unpack correctly.
    UnpackTestCase tests[] = {
        {"0x000000  => {0x00, 0x0, 0x00}",   0x000000,  0x00,  0x0, 0x00},
        {"0x074002  => {0x01, 0xd, 0x02}",   0x074002,  0x01,  0xd, 0x02},
        {"0x15c006  => {0x05, 0x7, 0x06}",   0x15c006,  0x05,  0x7, 0x06},
        {"0x1d0008  => {0x07, 0x4, 0x08}",   0x1d0008,  0x07,  0x4, 0x08},
        {"0x24000a  => {0x09, 0x0, 0x0a}",   0x24000a,  0x09,  0x0, 0x0a},
        {"0xfc003f  => {0x3f, 0x0, 0x3f}",   0xfc003f,  0x3f,  0x0, 0x3f},
        {"0x4000100 => {0x100, 0x0, 0x100}", 0x4000100, 0x100, 0x0, 0x100},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(UnpackTestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        UnpackTestCase tc = tests[i];
        int lower, upper, lerp;
        sktests::decode_packed_coordinates_and_weight(tc.input, &lower, &upper, &lerp);

        REPORTER_ASSERT(r, lower == tc.expectedLowerBound,
                "%s lower %x != %x", tc.name.c_str(), lower, tc.expectedLowerBound);
        REPORTER_ASSERT(r, lerp == tc.expectedLerp,
                "%s lerp %x != %x", tc.name.c_str(), lerp, tc.expectedLerp);
        REPORTER_ASSERT(r, upper == tc.expectedUpperBound,
                "%s upper %x != %x", tc.name.c_str(), upper, tc.expectedUpperBound);
        // Make sure our helpers work as expected.
        SkASSERT(tc.expectedLowerBound == highBits(tc.input));
        SkASSERT(tc.expectedLerp == middleBits(tc.input));
        SkASSERT(tc.expectedUpperBound == lowBits(tc.input));
    }
}