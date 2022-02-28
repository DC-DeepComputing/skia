
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_ivec_b() {
    int one = int(colorGreen.y);
    const int two = 2;
    ivec4 one_splat = ivec4(one);
    ivec4 one_compound = ivec4(one, one, one, one);
    bool ok = true;
    ok = ok && ivec4(-1) == -one_splat;
    ok = ok && ivec4(-1, -1, -1, -1) == -one_splat;
    ok = ok && ivec4(-1) == -one_compound;
    ok = ok && ivec4(-1, -1, -1, -1) == -one_compound;
    ok = ok && ivec4(-1) == -one_splat;
    ok = ok && ivec4(-1, -1, -1, -1) == -one_splat;
    ok = ok && ivec4(-1) == -one_compound;
    ok = ok && ivec4(-1, -1, -1, -1) == -one_compound;
    ok = ok && -ivec2(-one, one + one) == -ivec2(one - two, 2);
    return ok;
}
bool test_mat_b() {
    float one = colorGreen.y;
    mat3 one_diagonal = mat3(one);
    mat3 one_compound = mat3(one, 0.0, 0.0, 0.0, one, 0.0, 0.0, 0.0, one);
    bool ok = true;
    ok = ok && mat3(-1.0) == -one_diagonal;
    ok = ok && mat3(-1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0) == -one_diagonal;
    ok = ok && mat3(-1.0) == -one_compound;
    ok = ok && mat3(-1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0) == -one_compound;
    ok = ok && mat3(-1.0) == -one_diagonal;
    ok = ok && mat3(-1.0, -0.0, -0.0, -0.0, -1.0, -0.0, -0.0, -0.0, -1.0) == -one_diagonal;
    ok = ok && mat3(-1.0) == -one_compound;
    ok = ok && mat3(-1.0, -0.0, -0.0, -0.0, -1.0, -0.0, -0.0, -0.0, -1.0) == -one_compound;
    return ok;
}
vec4 main() {
    float _0_one = colorGreen.y;
    float _1_two = 2.0;
    vec4 _2_one_splat = vec4(_0_one);
    vec4 _3_one_compound = vec4(_0_one, _0_one, _0_one, _0_one);
    bool _4_ok = true;
    _4_ok = _4_ok && vec4(-1.0) == -_2_one_splat;
    _4_ok = _4_ok && vec4(-1.0, -1.0, -1.0, -1.0) == -_2_one_splat;
    _4_ok = _4_ok && vec4(-1.0) == -_3_one_compound;
    _4_ok = _4_ok && vec4(-1.0, -1.0, -1.0, -1.0) == -_3_one_compound;
    _4_ok = _4_ok && vec4(-1.0) == -_2_one_splat;
    _4_ok = _4_ok && vec4(-1.0, -1.0, -1.0, -1.0) == -_2_one_splat;
    _4_ok = _4_ok && vec4(-1.0) == -_3_one_compound;
    _4_ok = _4_ok && vec4(-1.0, -1.0, -1.0, -1.0) == -_3_one_compound;
    _4_ok = _4_ok && vec4(-1.0) == -_3_one_compound;
    _4_ok = _4_ok && -vec4(_1_two) == vec4(-_1_two, vec3(-_1_two));
    _4_ok = _4_ok && -vec2(-_0_one, _0_one + _0_one) == -vec2(_0_one - _1_two, _1_two);
    return (_4_ok && test_ivec_b()) && test_mat_b() ? colorGreen : colorRed;
}
