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
half3 blend_set_color_saturation_helper_Qh3h3h(half3 minMidMax, half sat) {
    if (minMidMax.x < minMidMax.z) {
        return half3(0.0h, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return half3(0.0h);
    }
}
half4 blend_hslc_h4h4h4hb(half4 src, half4 dst, half flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 _8_l = mix(sda, dsa, flip);
    half3 _9_r = mix(dsa, sda, flip);
    if (saturate) {
        half3 _10_blend_set_color_saturation;
        half _11_sat = max(max(_9_r.x, _9_r.y), _9_r.z) - min(min(_9_r.x, _9_r.y), _9_r.z);
        if (_8_l.x <= _8_l.y) {
            if (_8_l.y <= _8_l.z) {
                _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l, _11_sat);
            } else if (_8_l.x <= _8_l.z) {
                _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l.xzy, _11_sat).xzy;
            } else {
                _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l.zxy, _11_sat).yzx;
            }
        } else if (_8_l.x <= _8_l.z) {
            _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l.yxz, _11_sat).yxz;
        } else if (_8_l.y <= _8_l.z) {
            _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l.yzx, _11_sat).zxy;
        } else {
            _10_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(_8_l.zyx, _11_sat).zyx;
        }
        _8_l = _10_blend_set_color_saturation;
        _9_r = dsa;
    }
    half3 _12_blend_set_color_luminance;
    half _13_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), _9_r);
    half3 _14_result = (_13_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), _8_l)) + _8_l;
    half _15_minComp = min(min(_14_result.x, _14_result.y), _14_result.z);
    half _16_maxComp = max(max(_14_result.x, _14_result.y), _14_result.z);
    if (_15_minComp < 0.0h && _13_lum != _15_minComp) {
        _14_result = _13_lum + (_14_result - _13_lum) * (_13_lum / (_13_lum - _15_minComp));
    }
    if (_16_maxComp > alpha && _16_maxComp != _13_lum) {
        _12_blend_set_color_luminance = _13_lum + ((_14_result - _13_lum) * (alpha - _13_lum)) / (_16_maxComp - _13_lum);
    } else {
        _12_blend_set_color_luminance = _14_result;
    }
    return half4((((_12_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4hb(_uniforms.src, _uniforms.dst, 0.0h, false);
    return _out;
}
