#version 400
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float guarded_divide_Qhhh(float n, float d);
float guarded_divide_Qhhh(float n, float d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
}
void main() {
    float _0_dyTerm = dst.w == dst.x ? dst.w : 0.0;
    float _1_delta = abs(src.x) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, guarded_divide_Qhhh((dst.w - dst.x) * src.w, src.x)) : _0_dyTerm;
    float _2_dyTerm = dst.w == dst.y ? dst.w : 0.0;
    float _3_delta = abs(src.y) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, guarded_divide_Qhhh((dst.w - dst.y) * src.w, src.y)) : _2_dyTerm;
    float _4_dyTerm = dst.w == dst.z ? dst.w : 0.0;
    float _5_delta = abs(src.z) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, guarded_divide_Qhhh((dst.w - dst.z) * src.w, src.z)) : _4_dyTerm;
    sk_FragColor = vec4((_1_delta * src.w + src.x * (1.0 - dst.w)) + dst.x * (1.0 - src.w), (_3_delta * src.w + src.y * (1.0 - dst.w)) + dst.y * (1.0 - src.w), (_5_delta * src.w + src.z * (1.0 - dst.w)) + dst.z * (1.0 - src.w), src.w + (1.0 - src.w) * dst.w);
}
