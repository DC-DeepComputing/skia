
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler s;
layout (binding = 1) uniform texture2D t;
layout (location = 1) in vec2 c;
vec4 doTextureStuff_h4ssT(sampler s, texture2D t) {
    return texture(makeSampler2D(t, s), c).zyxw;
}
void main() {
    sk_FragColor = doTextureStuff_h4ssT(s, t);
}
