
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
    float _0_dxScale = float(dst.x == 0.0 ? 0 : 1);
    float _1_delta = _0_dxScale * min(dst.w, abs(src.w - src.x) >= sk_PrivkMinNormalHalf ? guarded_divide_Qhhh(dst.x * src.w, src.w - src.x) : dst.w);
    float _2_dxScale = float(dst.y == 0.0 ? 0 : 1);
    float _3_delta = _2_dxScale * min(dst.w, abs(src.w - src.y) >= sk_PrivkMinNormalHalf ? guarded_divide_Qhhh(dst.y * src.w, src.w - src.y) : dst.w);
    float _4_dxScale = float(dst.z == 0.0 ? 0 : 1);
    float _5_delta = _4_dxScale * min(dst.w, abs(src.w - src.z) >= sk_PrivkMinNormalHalf ? guarded_divide_Qhhh(dst.z * src.w, src.w - src.z) : dst.w);
    sk_FragColor = vec4((_1_delta * src.w + src.x * (1.0 - dst.w)) + dst.x * (1.0 - src.w), (_3_delta * src.w + src.y * (1.0 - dst.w)) + dst.y * (1.0 - src.w), (_5_delta * src.w + src.z * (1.0 - dst.w)) + dst.z * (1.0 - src.w), src.w + (1.0 - src.w) * dst.w);
}
