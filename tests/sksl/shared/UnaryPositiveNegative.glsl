
out vec4 sk_FragColor;
uniform vec4 colorWhite;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test_iscalar_b() {
    int x = int(colorWhite.x);
    x = -x;
    return x == -1;
}
bool test_fvec_b() {
    vec2 x = colorWhite.xy;
    x = -x;
    return x == vec2(-1.0);
}
bool test_ivec_b() {
    ivec2 x = ivec2(int(colorWhite.x));
    x = -x;
    return x == ivec2(-1);
}
bool test_mat2_b() {
    mat2 x = testMatrix2x2;
    x = -x;
    return x == -testMatrix2x2;
}
bool test_mat3_b() {
    mat3 x = testMatrix3x3;
    x = -x;
    return x == -testMatrix3x3;
}
bool test_mat4_b() {
    mat4 x = testMatrix4x4;
    x = -x;
    return x == -testMatrix4x4;
}
vec4 main() {
    float _0_x = colorWhite.x;
    _0_x = -_0_x;
    return (((((_0_x == -1.0 && test_iscalar_b()) && test_fvec_b()) && test_ivec_b()) && test_mat2_b()) && test_mat3_b()) && test_mat4_b() ? colorGreen : colorRed;
}
