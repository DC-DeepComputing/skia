
out vec4 sk_FragColor;
layout (binding = 1) testBlockA {
    vec2 x;
};
layout (binding = 2) testBlockB {
    vec2 y;
};
void main() {
    sk_FragColor = vec4(x, y);
}
