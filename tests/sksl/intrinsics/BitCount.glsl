
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    sk_FragColor.x = float(bitCount(a)) + float(bitCount(b));
    sk_FragColor.xy = vec2(bitCount(ivec2(a))) + vec2(bitCount(uvec2(b)));
    sk_FragColor.xyz = vec3(bitCount(ivec3(a))) + vec3(bitCount(uvec3(b)));
    sk_FragColor = vec4(bitCount(ivec4(a))) + vec4(bitCount(uvec4(b)));
}
