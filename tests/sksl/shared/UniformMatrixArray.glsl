
out vec4 sk_FragColor;
uniform mat3 testArray[3];
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    for (int index = 0;index < 3; ++index) {
        if (testArray[index][index][index] != 1.0) {
            return colorRed;
        }
    }
    return colorGreen;
}
