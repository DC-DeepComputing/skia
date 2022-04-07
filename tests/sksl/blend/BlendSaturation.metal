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
half4 blend_hslc_h4h4h4bb(half4 src, half4 dst, bool flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = flip ? dsa : sda;
    half3 r = flip ? sda : dsa;
    if (saturate) {
        half3 _2_hueLumColor = l;
        half3 _3_midPt = half3(0.0h);
        half3 _4_satPt = half3(0.0h);
        if (_2_hueLumColor.x <= _2_hueLumColor.y) {
            if (_2_hueLumColor.y <= _2_hueLumColor.z) {
                _2_hueLumColor -= _2_hueLumColor.xxx;
                _3_midPt.y = (_4_satPt.z = 1.0h);
            } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
                _2_hueLumColor = _2_hueLumColor.xzy - _2_hueLumColor.xxx;
                _3_midPt.z = (_4_satPt.y = 1.0h);
            } else {
                _2_hueLumColor = _2_hueLumColor.zxy - _2_hueLumColor.zzz;
                _3_midPt.x = (_4_satPt.y = 1.0h);
            }
        } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yxz - _2_hueLumColor.yyy;
            _3_midPt.x = (_4_satPt.z = 1.0h);
        } else if (_2_hueLumColor.y <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yzx - _2_hueLumColor.yyy;
            _3_midPt.z = (_4_satPt.x = 1.0h);
        } else {
            _2_hueLumColor = _2_hueLumColor.zyx - _2_hueLumColor.zzz;
            _3_midPt.y = (_4_satPt.x = 1.0h);
        }
        if (_2_hueLumColor.z > 0.0h) {
            half _5_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
            half _6_mid = _2_hueLumColor.y / _2_hueLumColor.z;
            _2_hueLumColor = _5_sat * (half3(_6_mid) * _3_midPt + _4_satPt);
        }
        l = _2_hueLumColor;
        r = dsa;
    }
    half _7_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _8_result = (_7_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    half _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0h && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (_7_lum - _9_minComp));
    }
    if (_10_maxComp > alpha && _10_maxComp != _7_lum) {
        _8_result = _7_lum + ((_8_result - _7_lum) * (alpha - _7_lum)) / (_10_maxComp - _7_lum);
    }
    return half4((((_8_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4bb(_uniforms.src, _uniforms.dst, true, true);
    return _out;
}
