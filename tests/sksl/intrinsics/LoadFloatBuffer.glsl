
out vec4 sk_FragColor;
layout (binding = 0, set = 0) buffer FSGradientBuffer {
    float[] fsGradientBuffer;
};
void avoidInline_vf(out float f) {
    f = fsGradientBuffer[0];
}
vec4 main() {
    float f = 0.0;
    avoidInline_vf(f);
    return vec4(f);
}
