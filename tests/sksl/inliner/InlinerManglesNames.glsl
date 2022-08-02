
out vec4 sk_FragColor;
uniform vec4 color;
vec4 main() {
    float a = ((color.x) * (color.y) + (color.z));
    float b = ((color.y) * (color.z) + (color.w));
    float c = ((color.z) * (color.w) + (color.x));
    return vec4(a, b, c * c, a * (b * c));
}
