
out vec4 sk_FragColor;
uniform mat3 test3x3Matrix;
uniform vec4 colorGreen;
uniform vec4 colorRed;
mat2 resizeMatrix_f22() {
    return mat2(test3x3Matrix);
}
vec4 main() {
    return resizeMatrix_f22() == mat2(1.0, 2.0, 4.0, 5.0) ? colorGreen : colorRed;
}
