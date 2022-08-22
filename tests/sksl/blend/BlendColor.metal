#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
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
half guarded_divide_Qhhh(half n, half d);
half4 blend_hslc_h4h4h4h2(half4 src, half4 dst, half2 flipSat);
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
half guarded_divide_Qhhh(half n, half d) {
    return false ? n / (d + 9.9999999392252903e-09h) : n / d;
}
half4 blend_hslc_h4h4h4h2(half4 src, half4 dst, half2 flipSat) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = bool(flipSat.x) ? dsa : sda;
    half3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        half _4_mn = min(min(l.x, l.y), l.z);
        half _5_mx = max(max(l.x, l.y), l.z);
        l = _5_mx > _4_mn ? ((l - _4_mn) * blend_color_saturation_Qhh3(r)) / (_5_mx - _4_mn) : half3(0.0h);
        r = dsa;
    }
    half _6_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _7_result = (_6_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _8_minComp = min(min(_7_result.x, _7_result.y), _7_result.z);
    half _9_maxComp = max(max(_7_result.x, _7_result.y), _7_result.z);
    if (_8_minComp < 0.0h && _6_lum != _8_minComp) {
        _7_result = _6_lum + (_7_result - _6_lum) * guarded_divide_Qhhh(_6_lum, _6_lum - _8_minComp);
    }
    if (_9_maxComp > alpha && _9_maxComp != _6_lum) {
        half3 _10_n = (_7_result - _6_lum) * (alpha - _6_lum);
        half _11_d = _9_maxComp - _6_lum;
        _7_result = _6_lum + (false ? _10_n / (_11_d + 9.9999999392252903e-09h) : _10_n / _11_d);
    }
    return half4((((_7_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4h2(_uniforms.src, _uniforms.dst, half2(0.0h));
    return _out;
}
