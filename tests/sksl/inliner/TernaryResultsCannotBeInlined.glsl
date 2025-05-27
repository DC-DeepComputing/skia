
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return bool(colorGreen.y) ? vec4(colorGreen.x, 1.0, colorGreen.zw) : vec4(colorRed.x, 0.0, colorRed.zw);
}
