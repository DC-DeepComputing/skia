
out vec4 sk_FragColor;
vec4 main() {
    float y[8];
    y[ivec2(1, 3)[(y[7] = y[3] , 4)]];
    return vec4(0.0);
}
