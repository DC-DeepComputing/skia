
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_hslc_h4h4h4bb(vec4 src, vec4 dst, bool flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = flip ? dsa : sda;
    vec3 r = flip ? sda : dsa;
    if (saturate) {
        vec3 _2_hueLumColor = l;
        vec3 _3_midPt = vec3(0.0);
        vec3 _4_satPt = vec3(0.0);
        if (_2_hueLumColor.x <= _2_hueLumColor.y) {
            if (_2_hueLumColor.y <= _2_hueLumColor.z) {
                _2_hueLumColor -= _2_hueLumColor.xxx;
                _3_midPt.y = (_4_satPt.z = 1.0);
            } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
                _2_hueLumColor = _2_hueLumColor.xzy - _2_hueLumColor.xxx;
                _3_midPt.z = (_4_satPt.y = 1.0);
            } else {
                _2_hueLumColor = _2_hueLumColor.zxy - _2_hueLumColor.zzz;
                _3_midPt.x = (_4_satPt.y = 1.0);
            }
        } else if (_2_hueLumColor.x <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yxz - _2_hueLumColor.yyy;
            _3_midPt.x = (_4_satPt.z = 1.0);
        } else if (_2_hueLumColor.y <= _2_hueLumColor.z) {
            _2_hueLumColor = _2_hueLumColor.yzx - _2_hueLumColor.yyy;
            _3_midPt.z = (_4_satPt.x = 1.0);
        } else {
            _2_hueLumColor = _2_hueLumColor.zyx - _2_hueLumColor.zzz;
            _3_midPt.y = (_4_satPt.x = 1.0);
        }
        if (_2_hueLumColor.z > 0.0) {
            float _5_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
            float _6_mid = _2_hueLumColor.y / _2_hueLumColor.z;
            _2_hueLumColor = _5_sat * (vec3(_6_mid) * _3_midPt + _4_satPt);
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
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hslc_h4h4h4bb(color, color.wwww, false, true);
    sk_FragColor *= blend_hslc_h4h4h4bb(color, color.wzyx, false, true);
}
