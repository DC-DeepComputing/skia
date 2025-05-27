
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    float _0_alpha = dst.w * src.w;
    vec3 _1_sda = src.xyz * dst.w;
    vec3 _2_dsa = dst.xyz * src.w;
    vec3 _3_l = _2_dsa;
    vec3 _4_r = _1_sda;
    float _5_lum = dot(vec3(0.3, 0.59, 0.11), _4_r);
    vec3 _6_result = (_5_lum - dot(vec3(0.3, 0.59, 0.11), _3_l)) + _3_l;
    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_8_maxComp > _0_alpha && _8_maxComp != _5_lum) {
        _6_result = _5_lum + ((_6_result - _5_lum) * (_0_alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    sk_FragColor = vec4((((_6_result + dst.xyz) - _2_dsa) + src.xyz) - _1_sda, (src.w + dst.w) - _0_alpha);
}
