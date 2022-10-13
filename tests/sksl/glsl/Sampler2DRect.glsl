
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2DRect t;
vec4 main() {
    return texture(t, coords);
}
