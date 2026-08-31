
out vec4 sk_FragColor;
layout (rgba8, binding = 0) uniform readonlyTexture2D t;
void main() {
    sk_FragColor = textureRead(t, uvec2(0u));
}
