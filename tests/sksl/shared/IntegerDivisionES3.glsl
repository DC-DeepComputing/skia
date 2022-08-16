
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int nine = int(colorGreen.w * 9.0);
    int three = int(colorRed.w * 3.0);
    return nine / three == 3 ? colorGreen : colorRed;
}
