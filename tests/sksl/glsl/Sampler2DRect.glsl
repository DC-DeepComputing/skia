
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2DRect t;
vec4 main() {
    vec4 c = vec4(1.0);
    c *= texture(t, coords);
    c *= textureGrad(t, coords, dFdx(coords), (u_skRTFlip.y * dFdy(coords)));
    return c;
}
