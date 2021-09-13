
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
vec4 main() {
    mat2 h22 = mat2(0.0, 5.0, 10.0, 15.0);
    mat4 h44 = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 5.5, 0.0, 0.0, 0.0, 0.0, 8.0);
    mat4x3 f43 = mat4x3(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    return (h22 == mat2(0.0, 5.0, 10.0, 15.0) && h44 == mat4(0.5, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 5.5, 0.0, 0.0, 0.0, 0.0, 8.0)) && f43 == mat4x3(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0) ? colorGreen : colorRed;
}
