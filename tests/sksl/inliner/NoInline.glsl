
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 multiplyByAlpha_h4h4(vec4 x) {
    return x * x.wwww;
}
vec4 main() {
    vec4 result = vec3(vec2(((colorGreen.w) * (colorGreen.y) + (colorGreen.x))), 0.0).zxzy;
    result = multiplyByAlpha_h4h4(result);
    return result;
}
