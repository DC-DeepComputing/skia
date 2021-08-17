
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const mat4 testMatrix4x4 = mat4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
    return (transpose(testMatrix2x2) == mat2(1.0, 3.0, 2.0, 4.0) && transpose(testMatrix3x3) == mat3(1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0)) && transpose(testMatrix4x4) == mat4(1.0, 5.0, 9.0, 13.0, 2.0, 6.0, 10.0, 14.0, 3.0, 7.0, 11.0, 15.0, 4.0, 8.0, 12.0, 16.0) ? colorGreen : colorRed;
}
