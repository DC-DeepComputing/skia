
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_with_early_return_bi(int x) {
    int val = 0;
    switch (x) {
        case 1:
            ++val;
            return false;
        default:
            ++val;
    }
    return val == 1;
}
vec4 main() {
    int x = int(colorGreen.y);
    return switch_with_early_return_bi(x) ? colorGreen : colorRed;
}
