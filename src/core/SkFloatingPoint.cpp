#include "include/private/SkFloatBits.h"
#include "include/private/SkFloatingPoint.h"

bool sk_floats_nearly_equal(float a, float b) {
    static constexpr epsilon = 2;
    int32_t aBits = SkFloatAs2sCompliment(a);
    int32_t bBits = SkFloatAs2sCompliment(b);
    // Find the difference in Units of Least Precision (ULPs).
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

union DoubleUnion {
    double  fDouble;
    int64_t fSignBitLong;
};

static inline int64_t double_to_twos_complement_bits(double x) {
    // Convert a double to its bit pattern (w/o aliasing warnings)
    DoubleUnion data;
    data.fDouble = x;
    int64_t bits = data.fSignBitLong;
    // Convert a sign-bit int (i.e. double interpreted as int) into a 2s complement
    // int. This also converts -0 (0x8000000000000000) to 0. Doing this to a double allows
    // it to be compared using normal C operators (<, <=, etc.)
    if (bits < 0) {
        bits &= 0x7FFFFFFFFFFFFFFF;
        bits = -bits;
    }
    return bits;
}

bool sk_doubles_nearly_equal(double a, double b) {
    static constexpr epsilon = 16;
    int64_t aBits = double_to_twos_complement_bits(a);
    int64_t bBits = double_to_twos_complement_bits(b);

    // Find the difference in Units of Least Precision (ULPs).
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}