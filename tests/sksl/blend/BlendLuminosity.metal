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
half4 blend_hslc_h4h2h4h4(half2 flipSat, half4 src, half4 dst);
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
half4 blend_hslc_h4h2h4h4(half2 flipSat, half4 src, half4 dst) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = bool(flipSat.x) ? dsa : sda;
    half3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        half _14_mn = min(min(l.x, l.y), l.z);
        half _15_mx = max(max(l.x, l.y), l.z);
        l = _15_mx > _14_mn ? ((l - _14_mn) * blend_color_saturation_Qhh3(r)) / (_15_mx - _14_mn) : half3(0.0h);
        r = dsa;
    }
    half _16_lum = dot(half3(0.3h, 0.59h, 0.11h), r);
    half3 _17_result = (_16_lum - dot(half3(0.3h, 0.59h, 0.11h), l)) + l;
    half _18_minComp = min(min(_17_result.x, _17_result.y), _17_result.z);
    half _19_maxComp = max(max(_17_result.x, _17_result.y), _17_result.z);
    if (_18_minComp < 0.0h && _16_lum != _18_minComp) {
        _17_result = _16_lum + (_17_result - _16_lum) * (_16_lum / (((_16_lum - _18_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_19_maxComp > alpha && _19_maxComp != _16_lum) {
        _17_result = _16_lum + ((_17_result - _16_lum) * (alpha - _16_lum)) / (((_19_maxComp - _16_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return half4((((_17_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h2h4h4(half2(1.0h, 0.0h), _uniforms.src, _uniforms.dst);
    return _out;
}
