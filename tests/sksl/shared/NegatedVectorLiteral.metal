#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool test_int_b() {
    int one = 1;
    const int two = 2;
    int4 result;
    result.x = 1;
    result.y = 1;
    result.z = 1;
    result.w = int(all(-int2(-one, one + one) == -int2(one - two, 2)) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float _0_one = 1.0f;
    float _1_two = 2.0f;
    float4 _2_result;
    _2_result.x = 1.0f;
    _2_result.y = 1.0f;
    _2_result.z = float(all(-half4(half(_1_two)) == half4(half(-_1_two), half3(half(-_1_two)))) ? 1 : 0);
    _2_result.w = float(all(half2(1.0h, -2.0h) == -half2(half(_0_one - _1_two), half(_1_two))) ? 1 : 0);
    _out.sk_FragColor = bool(((_2_result.x * _2_result.y) * _2_result.z) * _2_result.w) && test_int_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
