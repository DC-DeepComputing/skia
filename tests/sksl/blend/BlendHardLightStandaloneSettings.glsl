
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_overlay_h4h4h4(vec4 src, vec4 dst);
vec4 blend_overlay_h4h4h4(vec4 src, vec4 dst) {
    vec4 result = vec4(2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x), 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y), 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
void main() {
    sk_FragColor = blend_overlay_h4h4h4(dst, src);
}
