
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 main() {
    vec4 result = vec4(0.0);
    if (bool(colorWhite.x) && bool(colorWhite.y)) {
        result.y = 1.0;
    }
    if (bool(colorWhite.y) || bool(colorWhite.x)) {
        result.w = 1.0;
    }
    return result;
}
