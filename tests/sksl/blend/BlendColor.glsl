#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec3 blend_set_color_saturation_helper_Qh3h3h(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        return vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return vec3(0.0);
    }
}
vec4 blend_hslc_h4h4h4hb(vec4 src, vec4 dst, float flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 _8_l = mix(sda, dsa, flip);
    vec3 _9_r = mix(dsa, sda, flip);
    if (saturate) {
        vec3 _10_blend_set_color_saturation;
        float _11_sat = max(max(_9_r.x, _9_r.y), _9_r.z) - min(min(_9_r.x, _9_r.y), _9_r.z);
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
    vec3 _12_blend_set_color_luminance;
    float _13_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _9_r);
    vec3 _14_result = (_13_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _8_l)) + _8_l;
    float _15_minComp = min(min(_14_result.x, _14_result.y), _14_result.z);
    float _16_maxComp = max(max(_14_result.x, _14_result.y), _14_result.z);
    if (_15_minComp < 0.0 && _13_lum != _15_minComp) {
        _14_result = _13_lum + (_14_result - _13_lum) * (_13_lum / (_13_lum - _15_minComp));
    }
    if (_16_maxComp > alpha && _16_maxComp != _13_lum) {
        _12_blend_set_color_luminance = _13_lum + ((_14_result - _13_lum) * (alpha - _13_lum)) / (_16_maxComp - _13_lum);
    } else {
        _12_blend_set_color_luminance = _14_result;
    }
    return vec4((((_12_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h4h4hb(src, dst, 0.0, false);
}
