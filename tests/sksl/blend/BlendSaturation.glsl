#version 400
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float blend_color_saturation_Qhh3(vec3 color);
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
void main() {
    float _0_alpha = dst.w * src.w;
    vec3 _1_sda = src.xyz * dst.w;
    vec3 _2_dsa = dst.xyz * src.w;
    vec3 _3_l = _2_dsa;
    vec3 _4_r = _1_sda;
    {
        float _5_mn = min(min(_3_l.x, _3_l.y), _3_l.z);
        float _6_mx = max(max(_3_l.x, _3_l.y), _3_l.z);
        _3_l = _6_mx > _5_mn ? ((_3_l - _5_mn) * blend_color_saturation_Qhh3(_4_r)) / (_6_mx - _5_mn) : vec3(0.0);
        _4_r = _2_dsa;
    }
    float _7_lum = dot(vec3(0.3, 0.59, 0.11), _4_r);
    vec3 _8_result = (_7_lum - dot(vec3(0.3, 0.59, 0.11), _3_l)) + _3_l;
    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (((_7_lum - _9_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_10_maxComp > _0_alpha && _10_maxComp != _7_lum) {
        _8_result = _7_lum + ((_8_result - _7_lum) * (_0_alpha - _7_lum)) / (((_10_maxComp - _7_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    sk_FragColor = vec4((((_8_result + dst.xyz) - _2_dsa) + src.xyz) - _1_sda, (src.w + dst.w) - _0_alpha);
}
