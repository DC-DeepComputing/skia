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
half4 blend_hslc_h4h4h4hb(half4 src, half4 dst, half flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = mix(sda, dsa, flip);
    half3 r = mix(dsa, sda, flip);
    if (saturate) {
        half3 _2_blend_set_color_saturation;
        half3 _3_hueLumColor = l;
        half _4_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
        half3x3 _5_unpermute = half3x3(1.0h);
        if (_3_hueLumColor.x > _3_hueLumColor.y) {
            _3_hueLumColor.xy = _3_hueLumColor.yx;
            _5_unpermute = half3x3(_5_unpermute[0].yxz, _5_unpermute[1].yxz, _5_unpermute[2].yxz);
        }
        if (_3_hueLumColor.x > _3_hueLumColor.z) {
            _3_hueLumColor.xz = _3_hueLumColor.zx;
            _5_unpermute = half3x3(_5_unpermute[0].zyx, _5_unpermute[1].zyx, _5_unpermute[2].zyx);
        }
        if (_3_hueLumColor.y > _3_hueLumColor.z) {
            _3_hueLumColor.yz = _3_hueLumColor.zy;
            _5_unpermute = half3x3(_5_unpermute[0].xzy, _5_unpermute[1].xzy, _5_unpermute[2].xzy);
        }
        if (_3_hueLumColor.z > _3_hueLumColor.x) {
            _3_hueLumColor.yz = _3_hueLumColor.yz - _3_hueLumColor.xx;
            _2_blend_set_color_saturation = half3(0.0h, (_4_sat * _3_hueLumColor.y) / _3_hueLumColor.z, _4_sat) * _5_unpermute;
        } else {
            _2_blend_set_color_saturation = half3(0.0h);
        }
        l = _2_blend_set_color_saturation;
        r = dsa;
    }
    half3 _6_blend_set_color_luminance;
    half _7_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _8_result = (_7_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    half _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0h && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (_7_lum - _9_minComp));
    }
    if (_10_maxComp > alpha && _10_maxComp != _7_lum) {
        _6_blend_set_color_luminance = _7_lum + ((_8_result - _7_lum) * (alpha - _7_lum)) / (_10_maxComp - _7_lum);
    } else {
        _6_blend_set_color_luminance = _8_result;
    }
    return half4((((_6_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4hb(_uniforms.src, _uniforms.dst, 1.0h, false);
    return _out;
}
