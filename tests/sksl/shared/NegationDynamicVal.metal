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

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
bool test_ivec_b(Uniforms _uniforms) {
    int one = int(_uniforms.colorGreen.y);
    const int two = 2;
    int4 one_splat = int4(one);
    int4 one_compound = int4(one, one, one, one);
    bool ok = true;
    ok = ok && all(int4(-1) == -one_splat);
    ok = ok && all(int4(-1, -1, -1, -1) == -one_splat);
    ok = ok && all(int4(-1) == -one_compound);
    ok = ok && all(int4(-1, -1, -1, -1) == -one_compound);
    ok = ok && all(int4(-1) == -one_splat);
    ok = ok && all(int4(-1, -1, -1, -1) == -one_splat);
    ok = ok && all(int4(-1) == -one_compound);
    ok = ok && all(int4(-1, -1, -1, -1) == -one_compound);
    ok = ok && all(-int2(-one, one + one) == -int2(one - two, 2));
    return ok;
}
bool test_mat_b(Uniforms _uniforms) {
    float one = float(_uniforms.colorGreen.y);
    float3x3 one_diagonal = float3x3(one);
    float3x3 one_compound = float3x3(float3(one, 0.0, 0.0), float3(0.0, one, 0.0), float3(0.0, 0.0, one));
    bool ok = true;
    ok = ok && float3x3(-1.0) == -1.0 * one_diagonal;
    ok = ok && float3x3(float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0), float3(0.0, 0.0, -1.0)) == -1.0 * one_diagonal;
    ok = ok && float3x3(-1.0) == -1.0 * one_compound;
    ok = ok && float3x3(float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0), float3(0.0, 0.0, -1.0)) == -1.0 * one_compound;
    ok = ok && float3x3(-1.0) == -1.0 * one_diagonal;
    ok = ok && float3x3(float3(-1.0, -0.0, -0.0), float3(-0.0, -1.0, -0.0), float3(-0.0, -0.0, -1.0)) == -1.0 * one_diagonal;
    ok = ok && float3x3(-1.0) == -1.0 * one_compound;
    ok = ok && float3x3(float3(-1.0, -0.0, -0.0), float3(-0.0, -1.0, -0.0), float3(-0.0, -0.0, -1.0)) == -1.0 * one_compound;
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _0_one = float(_uniforms.colorGreen.y);
    float _1_two = 2.0;
    half4 _2_one_splat = half4(half(_0_one));
    half4 _3_one_compound = half4(half(_0_one), half(_0_one), half(_0_one), half(_0_one));
    bool _4_ok = true;
    _4_ok = _4_ok && all(half4(-1.0h) == -_2_one_splat);
    _4_ok = _4_ok && all(half4(-1.0h, -1.0h, -1.0h, -1.0h) == -_2_one_splat);
    _4_ok = _4_ok && all(half4(-1.0h) == -_3_one_compound);
    _4_ok = _4_ok && all(half4(-1.0h, -1.0h, -1.0h, -1.0h) == -_3_one_compound);
    _4_ok = _4_ok && all(half4(-1.0h) == -_2_one_splat);
    _4_ok = _4_ok && all(half4(-1.0h, -1.0h, -1.0h, -1.0h) == -_2_one_splat);
    _4_ok = _4_ok && all(half4(-1.0h) == -_3_one_compound);
    _4_ok = _4_ok && all(half4(-1.0h, -1.0h, -1.0h, -1.0h) == -_3_one_compound);
    _4_ok = _4_ok && all(half4(-1.0h) == -_3_one_compound);
    _4_ok = _4_ok && all(-half4(half(_1_two)) == half4(half(-_1_two), half3(half(-_1_two))));
    _4_ok = _4_ok && all(-half2(half(-_0_one), half(_0_one + _0_one)) == -half2(half(_0_one - _1_two), half(_1_two)));
    _out.sk_FragColor = (_4_ok && test_ivec_b(_uniforms)) && test_mat_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
