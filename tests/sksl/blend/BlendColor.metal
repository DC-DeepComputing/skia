#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const half sk_PrivkGuardedDivideEpsilon = half(false ? 1e-08 : 0.0);
constant const half sk_PrivkMinNormalHalf = 6.10351562e-05h;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    half3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    half3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    half3 _3_l = _1_sda;
    half3 _4_r = _2_dsa;
    half _5_lum = dot(half3(0.3h, 0.59h, 0.11h), _4_r);
    half3 _6_result = (_5_lum - dot(half3(0.3h, 0.59h, 0.11h), _3_l)) + _3_l;
    half _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    half _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0h && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_8_maxComp > _0_alpha && _8_maxComp != _5_lum) {
        _6_result = _5_lum + ((_6_result - _5_lum) * (_0_alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    _out.sk_FragColor = half4((((_6_result + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
