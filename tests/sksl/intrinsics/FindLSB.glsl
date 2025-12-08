
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    sk_FragColor.x = float(findLSB(a)) + float(findLSB(b));
    sk_FragColor.xy = vec2(findLSB(ivec2(a))) + vec2(findLSB(uvec2(b)));
    sk_FragColor.xyz = vec3(findLSB(ivec3(a))) + vec3(findLSB(uvec3(b)));
    sk_FragColor = vec4(findLSB(ivec4(a))) + vec4(findLSB(uvec4(b)));
}
