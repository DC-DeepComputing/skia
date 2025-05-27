
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct S {
    int i;
};
vec4 main() {
    return bool(colorGreen.y) ? colorGreen : colorRed;
}
