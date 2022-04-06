
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_hslc_h4h4h4hb(vec4 src, vec4 dst, float flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = mix(sda, dsa, flip);
    vec3 r = mix(dsa, sda, flip);
    if (saturate) {
        vec3 _2_blend_set_color_saturation;
        vec3 _3_hueLumColor = l;
        float _4_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
        mat3 _5_unpermute = mat3(1.0);
        if (_3_hueLumColor.x > _3_hueLumColor.y) {
            _3_hueLumColor.xy = _3_hueLumColor.yx;
            _5_unpermute = mat3(_5_unpermute[0].yxz, _5_unpermute[1].yxz, _5_unpermute[2].yxz);
        }
        if (_3_hueLumColor.x > _3_hueLumColor.z) {
            _3_hueLumColor.xz = _3_hueLumColor.zx;
            _5_unpermute = mat3(_5_unpermute[0].zyx, _5_unpermute[1].zyx, _5_unpermute[2].zyx);
        }
        if (_3_hueLumColor.y > _3_hueLumColor.z) {
            _3_hueLumColor.yz = _3_hueLumColor.zy;
            _5_unpermute = mat3(_5_unpermute[0].xzy, _5_unpermute[1].xzy, _5_unpermute[2].xzy);
        }
        if (_3_hueLumColor.z > _3_hueLumColor.x) {
            _3_hueLumColor.yz -= _3_hueLumColor.xx;
            _2_blend_set_color_saturation = vec3(0.0, (_4_sat * _3_hueLumColor.y) / _3_hueLumColor.z, _4_sat) * _5_unpermute;
        } else {
            _2_blend_set_color_saturation = vec3(0.0);
        }
        l = _2_blend_set_color_saturation;
        r = dsa;
    }
    vec3 _6_blend_set_color_luminance;
    float _7_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _8_result = (_7_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (_7_lum - _9_minComp));
    }
    if (_10_maxComp > alpha && _10_maxComp != _7_lum) {
        _6_blend_set_color_luminance = _7_lum + ((_8_result - _7_lum) * (alpha - _7_lum)) / (_10_maxComp - _7_lum);
    } else {
        _6_blend_set_color_luminance = _8_result;
    }
    return vec4((((_6_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hslc_h4h4h4hb(color, color.wwww, 0.0, true);
    sk_FragColor *= blend_hslc_h4h4h4hb(color, color.wzyx, 0.0, true);
}
