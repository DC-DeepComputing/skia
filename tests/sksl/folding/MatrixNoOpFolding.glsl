
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_mat3_mat3_b() {
    mat3 m;
    mat3 mm;
    const mat3 z = mat3(0.0);
    m = testMatrix3x3;
    m = testMatrix3x3;
    m = -m;
    mm = mat3(0.0);
    mm = mat3(0.0);
    return m == -testMatrix3x3 && mm == z;
}
bool test_mat4_mat4_b() {
    mat4 testMatrix4x4 = mat4(testInputs, testInputs, testInputs, testInputs);
    mat4 m;
    mat4 mm;
    const mat4 z = mat4(0.0);
    m = testMatrix4x4;
    m = testMatrix4x4;
    m = -m;
    mm = mat4(0.0);
    mm = mat4(0.0);
    return m == -testMatrix4x4 && mm == z;
}
bool test_mat2_vec2_b() {
    vec2 v;
    vec2 vv;
    v = testInputs.xy;
    v = testInputs.xy;
    vv = vec2(0.0);
    vv = vec2(0.0);
    return v == testInputs.xy && vv == vec2(0.0, 0.0);
}
bool test_mat2_scal_b() {
    mat2 m;
    mat2 mm;
    const mat2 z = mat2(0.0);
    const mat2 s = mat2(vec4(1.0));
    float scalar = testInputs.x;
    m = testMatrix2x2;
    m = testMatrix2x2;
    if (m != testMatrix2x2) return false;
    if (m != testMatrix2x2) return false;
    if (m != testMatrix2x2) return false;
    m = -m;
    if (m != -testMatrix2x2) return false;
    mm = mat2(0.0);
    mm = mat2(0.0);
    if (mm != z) return false;
    m = mat2(scalar);
    m = mat2(scalar);
    if (m != mat2(scalar)) return false;
    m = scalar / s;
    if (m != mat2(scalar, scalar, scalar, scalar)) return false;
    m = scalar + z;
    m = z + scalar;
    if (m != mat2(scalar, scalar, scalar, scalar)) return false;
    m = scalar - z;
    m = z - scalar;
    if (m != -mat2(scalar, scalar, scalar, scalar)) return false;
    mm = mat2(0.0);
    mm = mat2(0.0);
    return mm == z;
}
bool test_mat3_vec3_b() {
    vec3 v;
    vec3 vv;
    v = testInputs.xyz;
    v = testInputs.xyz;
    vv = vec3(0.0);
    vv = vec3(0.0);
    return v == testInputs.xyz && vv == vec3(0.0, 0.0, 0.0);
}
bool test_mat4_vec4_b() {
    vec4 v;
    vec4 vv;
    v = testInputs;
    v = testInputs;
    vv = vec4(0.0);
    vv = vec4(0.0);
    return v == testInputs && vv == vec4(0.0, 0.0, 0.0, 0.0);
}
vec4 main() {
    mat2 _0_m;
    mat2 _1_mm;
    const mat2 _3_z = mat2(0.0);
    _0_m = testMatrix2x2;
    _0_m = testMatrix2x2;
    _0_m = -_0_m;
    _1_mm = mat2(0.0);
    _1_mm = mat2(0.0);
    return ((((((_0_m == -testMatrix2x2 && _1_mm == _3_z) && test_mat3_mat3_b()) && test_mat4_mat4_b()) && test_mat2_vec2_b()) && test_mat3_vec3_b()) && test_mat4_vec4_b()) && test_mat2_scal_b() ? colorGreen : colorRed;
}
