
out vec4 sk_FragColor;
uniform vec4 color;
float blend_color_saturation_Qhh3(vec3 color);
float guarded_divide_Qhhh(float n, float d);
vec4 blend_hslc_h4h4h4h2(vec4 src, vec4 dst, vec2 flipSat);
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
float guarded_divide_Qhhh(float n, float d) {
    return false ? n / (d + 9.9999999392252903e-09) : n / d;
}
vec4 blend_hslc_h4h4h4h2(vec4 src, vec4 dst, vec2 flipSat) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = bool(flipSat.x) ? dsa : sda;
    vec3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        float _4_mn = min(min(l.x, l.y), l.z);
        float _5_mx = max(max(l.x, l.y), l.z);
        l = _5_mx > _4_mn ? ((l - _4_mn) * blend_color_saturation_Qhh3(r)) / (_5_mx - _4_mn) : vec3(0.0);
        r = dsa;
    }
    float _6_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _7_result = (_6_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _8_minComp = min(min(_7_result.x, _7_result.y), _7_result.z);
    float _9_maxComp = max(max(_7_result.x, _7_result.y), _7_result.z);
    if (_8_minComp < 0.0 && _6_lum != _8_minComp) {
        _7_result = _6_lum + (_7_result - _6_lum) * guarded_divide_Qhhh(_6_lum, _6_lum - _8_minComp);
    }
    if (_9_maxComp > alpha && _9_maxComp != _6_lum) {
        vec3 _10_n = (_7_result - _6_lum) * (alpha - _6_lum);
        float _11_d = _9_maxComp - _6_lum;
        _7_result = _6_lum + (false ? _10_n / (_11_d + 9.9999999392252903e-09) : _10_n / _11_d);
    }
    return vec4((((_7_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hslc_h4h4h4h2(color, color.wwww, vec2(0.0, 1.0));
    sk_FragColor *= blend_hslc_h4h4h4h2(color, color.wzyx, vec2(0.0, 1.0));
}
