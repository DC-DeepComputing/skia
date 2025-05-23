
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    vec4 _0_result = vec4(2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x), 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y), 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z), dst.w + (1.0 - dst.w) * src.w);
    _0_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
    sk_FragColor = _0_result;
}
