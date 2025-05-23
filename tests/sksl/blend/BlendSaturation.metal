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
half blend_color_saturation_Qhh3(half3 color);
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    half3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    half3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    half3 _3_l = _2_dsa;
    half3 _4_r = _1_sda;
    {
        half _5_mn = min(min(_3_l.x, _3_l.y), _3_l.z);
        half _6_mx = max(max(_3_l.x, _3_l.y), _3_l.z);
        _3_l = _6_mx > _5_mn ? ((_3_l - _5_mn) * blend_color_saturation_Qhh3(_4_r)) / (_6_mx - _5_mn) : half3(0.0h);
        _4_r = _2_dsa;
    }
    half _7_lum = dot(half3(0.3h, 0.59h, 0.11h), _4_r);
    half3 _8_result = (_7_lum - dot(half3(0.3h, 0.59h, 0.11h), _3_l)) + _3_l;
    half _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    half _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0h && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (((_7_lum - _9_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_10_maxComp > _0_alpha && _10_maxComp != _7_lum) {
        _8_result = _7_lum + ((_8_result - _7_lum) * (_0_alpha - _7_lum)) / (((_10_maxComp - _7_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    _out.sk_FragColor = half4((((_8_result + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
