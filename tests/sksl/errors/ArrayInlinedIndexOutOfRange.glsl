
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const int values[3] = int[3](1, 1, 1);
int indexArray_ii(int index) {
    return values[index];
}
vec4 main() {
    (indexArray_ii(0) + indexArray_ii(1)) + indexArray_ii(2);
    indexArray_ii(-1) + indexArray_ii(3);
    return colorGreen;
}
