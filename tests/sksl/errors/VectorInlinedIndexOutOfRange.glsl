
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const ivec3 values = ivec3(1);
int indexVector_ii(int index) {
    return values[index];
}
vec4 main() {
    (indexVector_ii(0) + indexVector_ii(1)) + indexVector_ii(2);
    indexVector_ii(-1) + indexVector_ii(3);
    return colorGreen;
}
