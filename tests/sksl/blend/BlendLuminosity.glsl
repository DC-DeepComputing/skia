#version 400
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float blend_color_saturation_Qhh3(vec3 color);
vec4 blend_hslc_h4h2h4h4(vec2 flipSat, vec4 src, vec4 dst);
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
vec4 blend_hslc_h4h2h4h4(vec2 flipSat, vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = bool(flipSat.x) ? dsa : sda;
    vec3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        float _14_mn = min(min(l.x, l.y), l.z);
        float _15_mx = max(max(l.x, l.y), l.z);
        l = _15_mx > _14_mn ? ((l - _14_mn) * blend_color_saturation_Qhh3(r)) / (_15_mx - _14_mn) : vec3(0.0);
        r = dsa;
    }
    float _16_lum = dot(vec3(0.3, 0.59, 0.11), r);
    vec3 _17_result = (_16_lum - dot(vec3(0.3, 0.59, 0.11), l)) + l;
    float _18_minComp = min(min(_17_result.x, _17_result.y), _17_result.z);
    float _19_maxComp = max(max(_17_result.x, _17_result.y), _17_result.z);
    if (_18_minComp < 0.0 && _16_lum != _18_minComp) {
        _17_result = _16_lum + (_17_result - _16_lum) * (_16_lum / (((_16_lum - _18_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_19_maxComp > alpha && _19_maxComp != _16_lum) {
        _17_result = _16_lum + ((_17_result - _16_lum) * (alpha - _16_lum)) / (((_19_maxComp - _16_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return vec4((((_17_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h2h4h4(vec2(1.0, 0.0), src, dst);
}
