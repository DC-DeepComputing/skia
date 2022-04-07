
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_src_in_h4h4h4(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst) {
    return dst * src.w;
}
vec4 blend_hslc_h4h4h4bb(vec4 src, vec4 dst, bool flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = flip ? dsa : sda;
    vec3 r = flip ? sda : dsa;
    if (saturate) {
        vec3 _2_hueLumColor = l;
        vec3 _3_swizzle = vec3(-1.0, 0.0, 1.0);
        if (_2_hueLumColor.x > _2_hueLumColor.y) {
            _2_hueLumColor = _2_hueLumColor.yxz;
            _3_swizzle = _3_swizzle.yxz;
        }
        if (_2_hueLumColor.x > _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.zyx;
            _3_swizzle = _3_swizzle.zyx;
        }
        if (_2_hueLumColor.y > _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.xzy;
            _3_swizzle = _3_swizzle.xzy;
        }
        _2_hueLumColor -= _2_hueLumColor.xxx;
        if (_2_hueLumColor.z > 0.0) {
            float _4_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
            float _5_mid = _4_sat * (_2_hueLumColor.y / _2_hueLumColor.z);
            mat3 _6_m = mat3(max(-_3_swizzle, 0.0), 1.0 - abs(_3_swizzle), max(_3_swizzle, 0.0));
            _2_hueLumColor = vec3(0.0, _5_mid, _4_sat) * _6_m;
        }
        l = _2_hueLumColor;
        r = dsa;
    }
    float _7_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _8_result = (_7_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (_7_lum - _9_minComp));
    }
    if (_10_maxComp > alpha && _10_maxComp != _7_lum) {
        _8_result = _7_lum + ((_8_result - _7_lum) * (alpha - _7_lum)) / (_10_maxComp - _7_lum);
    }
    return vec4((((_8_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst) {
    return blend_hslc_h4h4h4bb(src, dst, false, true);
}
float singleuse_h() {
    return 1.25;
}
float add_hhh(float a, float b) {
    float c = a + b;
    return c;
}
float mul_hhh(float a, float b) {
    return a * b;
}
float fma_hhhh(float a, float b, float c) {
    return add_hhh(mul_hhh(a, b), c);
}
void main() {
    sk_FragColor = vec4(fma_hhhh(color.x, color.y, color.z));
    sk_FragColor *= singleuse_h();
    sk_FragColor *= blend_src_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_dst_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wwww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wzyx);
}
