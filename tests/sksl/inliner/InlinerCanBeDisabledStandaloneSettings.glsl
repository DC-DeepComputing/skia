
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst);
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst) {
    float _24_alpha = dst.w * src.w;
    vec3 _25_sda = src.xyz * dst.w;
    vec3 _26_dsa = dst.xyz * src.w;
    vec3 _27_l = _25_sda;
    vec3 _28_r = _26_dsa;
    {
        float _29_mn = min(min(_27_l.x, _27_l.y), _27_l.z);
        float _30_mx = max(max(_27_l.x, _27_l.y), _27_l.z);
        _27_l = _30_mx > _29_mn ? ((_27_l - _29_mn) * (max(max(_28_r.x, _28_r.y), _28_r.z) - min(min(_28_r.x, _28_r.y), _28_r.z))) / (_30_mx - _29_mn) : vec3(0.0);
        _28_r = _26_dsa;
    }
    float _31_lum = dot(vec3(0.3, 0.59, 0.11), _28_r);
    vec3 _32_result = (_31_lum - dot(vec3(0.3, 0.59, 0.11), _27_l)) + _27_l;
    float _33_minComp = min(min(_32_result.x, _32_result.y), _32_result.z);
    float _34_maxComp = max(max(_32_result.x, _32_result.y), _32_result.z);
    if (_33_minComp < 0.0 && _31_lum != _33_minComp) {
        _32_result = _31_lum + (_32_result - _31_lum) * (_31_lum / (((_31_lum - _33_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_34_maxComp > _24_alpha && _34_maxComp != _31_lum) {
        _32_result = _31_lum + ((_32_result - _31_lum) * (_24_alpha - _31_lum)) / (((_34_maxComp - _31_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return vec4((((_32_result + dst.xyz) - _26_dsa) + src.xyz) - _25_sda, (src.w + dst.w) - _24_alpha);
}
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hue_h4h4h4(color, color.wwww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wzyx);
}
