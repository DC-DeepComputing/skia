#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right);
template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right);

array<int, 2> array_of_2x_int_from_int(thread const array<int, 2>& x) {
    return array<int, 2>{int{x[0]}, int{x[1]}};
}

array<float, 2> array_of_2x_float_from_float(thread const array<float, 2>& x) {
    return array<float, 2>{float{x[0]}, float{x[1]}};
}

template <typename T1, typename T2, size_t N>
bool operator==(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    for (size_t index = 0; index < N; ++index) {
        if (!all(left[index] == right[index])) {
            return false;
        }
    }
    return true;
}

template <typename T1, typename T2, size_t N>
bool operator!=(thread const array<T1, N>& left, thread const array<T2, N>& right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<int, 2> i2 = array<int, 2>{1, 2};
    array<int, 2> s2 = array<int, 2>{1, 2};
    array<float, 2> f2 = array<float, 2>{1.0, 2.0};
    array<float, 2> h2 = array<float, 2>{1.0, 2.0};
    i2 = array_of_2x_int_from_int(s2);
    s2 = array_of_2x_int_from_int(i2);
    f2 = array_of_2x_float_from_float(h2);
    h2 = array_of_2x_float_from_float(f2);
    const array<float, 2> cf2 = array<float, 2>{1.0, 2.0};
    _out.sk_FragColor = ((i2 == array_of_2x_int_from_int(s2) && f2 == array_of_2x_float_from_float(h2)) && i2 == array<int, 2>{1, 2}) && array_of_2x_float_from_float(h2) == cf2 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
