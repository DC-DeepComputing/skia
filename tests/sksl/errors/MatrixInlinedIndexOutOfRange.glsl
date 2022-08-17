
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const mat3 values = mat3(1.0);
vec3 indexMatrix_f3i(int index) {
    return values[index];
}
vec4 main() {
    (indexMatrix_f3i(0) + indexMatrix_f3i(1)) + indexMatrix_f3i(2);
    indexMatrix_f3i(-1) + indexMatrix_f3i(3);
    return colorGreen;
}
