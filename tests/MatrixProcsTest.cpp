#include "tests/Test.h"

#include "include/core/SkTypes.h"
#include "include/private/SkFixed.h"
#include "src/core/SkBitmapProcState.h"

struct PackTestCase {
    std::string name;
    SkFixed input;
    unsigned max;
    // For clamp, this is SK_Fixed1. For tile and mirror, this is SK_Fixed1 divided by the
    // appropriate dimension (e.g. the width of the pixmap for the X direction).
    SkFixed one;
    uint32_t expectedOutput;
};

uint32_t highBits(uint32_t rv) {
    const uint32_t HIGH_14_BITS = ((1 << 14) - 1) << 18;
    return (rv & HIGH_14_BITS) >> 18;
}

uint32_t middleBits(uint32_t rv) {
    const uint32_t MIDDLE_4_BITS = ((1 << 4) - 1) << 14;
    return (rv & MIDDLE_4_BITS) >> 14;
}

uint32_t lowBits(uint32_t rv) {
    const uint32_t LOW_14_BITS = ((1 << 14) - 1);
    return rv & LOW_14_BITS;
}

DEF_TEST(MatrixProcs_pack, r) {
    PackTestCase tests[] = {
            {"-9.00000  => {0x00, 0x0, 0x00}", SkIntToFixed(-9.0), 63, SK_Fixed1, 0x0},
            {"-4.00000  => {0x00, 0x0, 0x00}", SkIntToFixed(-4.0), 63, SK_Fixed1, 0x0},
            {"-3.70000  => {0x00, 0x4, 0x00}", SkFloatToFixed(-3.7), 63, SK_Fixed1, 0x10000},
            {"-1.80000  => {0x00, 0x3, 0x00}", SkFloatToFixed(-1.8), 63, SK_Fixed1, 0xc000},
            {"-0.50000  => {0x00, 0x8, 0x00}", SkFloatToFixed(-0.5), 63, SK_Fixed1, 0x20000},
            {"0.041656  => {0x00, 0x0, 0x01}", 0x000aaa, 63, SK_Fixed1, 0x000001},
            {"1.858322  => {0x01, 0xd, 0x02}", 0x01dbbb, 63, SK_Fixed1, 0x074002},
            {"3.674988  => {0x03, 0xa, 0x04}", 0x03accc, 63, SK_Fixed1, 0x0e8004},
            {"5.491669  => {0x05, 0x7, 0x06}", 0x057dde, 63, SK_Fixed1, 0x15c006},
            {"7.308334  => {0x07, 0x4, 0x08}", 0x074eef, 63, SK_Fixed1, 0x1d0008},
            {"9.000000  => {0x09, 0x0, 0x0a}", SkIntToFixed(9), 63, SK_Fixed1, 0x24000a},
            {"50.000000 => {0x32, 0x0, 0x33}", SkIntToFixed(50), 63, SK_Fixed1, 0xc80033},
            {"50.875000 => {0x32, 0xe, 0x33}", 0x32e000, 63, SK_Fixed1, 0xcb8033},
            {"62.123456 => {0x3e, 0x1, 0x3f}", 0x3e1f9a, 63, SK_Fixed1, 0xf8403f},
            {"62.999000 => {0x3e, 0xf, 0x3f}", 0x3effbe, 63, SK_Fixed1, 0xfbc03f},
            {"63.000000 => {0x3f, 0x0, 0x3f}", SkIntToFixed(63), 63, SK_Fixed1, 0xfc003f},
            {"65.000000 => {0x3f, 0x0, 0x3f}", SkIntToFixed(65), 63, SK_Fixed1, 0xfc003f},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(PackTestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        PackTestCase ptc = tests[i];
        uint32_t rv = sktests::pack_clamp(ptc.input, ptc.max, ptc.one);
        uint32_t exp = ptc.expectedOutput;
        REPORTER_ASSERT(r, rv == ptc.expectedOutput,
                        "%s %f %x | %x != %x | {%x, %x, %x} != {%x, %x, %x}",
                        ptc.name.c_str(), SkFixedToFloat(ptc.input), ptc.input, rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }

    //WARN: 'f = 2730, max = 63, one = 65536'
    //WARN: 'f = 121787, max = 63, one = 65536'
    //WARN: 'f = 240844, max = 63, one = 65536'
    //WARN: 'f = 359902, max = 63, one = 65536'
    //WARN: 'f = 478959, max = 63, one = 65536'
    //
    //WARN: 'f = -3004, max = 67, one = 65536'
    //WARN: 'f = 116053, max = 67, one = 65536'
    //WARN: 'f = 235110, max = 67, one = 65536'
    //WARN: 'f = 354167, max = 67, one = 65536'
    //WARN: 'f = 473224, max = 67, one = 65536'
    //WARN: 'f = 592281, max = 67, one = 65536'
    //WARN: 'f = 711338, max = 67, one = 65536'
    //WARN: 'f = 830395, max = 67, one = 65536'
    //WARN: 'f = 949452, max = 67, one = 65536'

}
