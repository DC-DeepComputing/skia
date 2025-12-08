
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    sk_FragColor.x = float(findMSB(a)) + float(findMSB(b));
    sk_FragColor.xy = vec2(findMSB(ivec2(a))) + vec2(findMSB(uvec2(b)));
    sk_FragColor.xyz = vec3(findMSB(ivec3(a))) + vec3(findMSB(uvec3(b)));
    sk_FragColor = vec4(findMSB(ivec4(a))) + vec4(findMSB(uvec4(b)));
}
