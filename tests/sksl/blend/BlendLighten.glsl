#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst);
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
void main() {
    vec4 _0_result = blend_src_over_h4h4h4(src, dst);
    _0_result.xyz = max(_0_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    sk_FragColor = _0_result;
}
