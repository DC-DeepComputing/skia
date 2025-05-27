
const float sk_PrivkGuardedDivideEpsilon = true ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float soft_light_component_Qhh2h2(vec2 s, vec2 d);
float soft_light_component_Qhh2h2(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        return (((d.x * d.x) * (s.y - 2.0 * s.x)) / (d.y + sk_PrivkGuardedDivideEpsilon) + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        return (((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x) / (DaSqd + sk_PrivkGuardedDivideEpsilon);
    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
void main() {
    float _0_dxScale = float(dst.x == 0.0 ? 0 : 1);
    float _1_delta = _0_dxScale * min(dst.w, abs(src.w - src.x) >= sk_PrivkMinNormalHalf ? (dst.x * src.w) / ((src.w - src.x) + sk_PrivkGuardedDivideEpsilon) : dst.w);
    float _2_dxScale = float(dst.y == 0.0 ? 0 : 1);
    float _3_delta = _2_dxScale * min(dst.w, abs(src.w - src.y) >= sk_PrivkMinNormalHalf ? (dst.y * src.w) / ((src.w - src.y) + sk_PrivkGuardedDivideEpsilon) : dst.w);
    float _4_dxScale = float(dst.z == 0.0 ? 0 : 1);
    float _5_delta = _4_dxScale * min(dst.w, abs(src.w - src.z) >= sk_PrivkMinNormalHalf ? (dst.z * src.w) / ((src.w - src.z) + sk_PrivkGuardedDivideEpsilon) : dst.w);
    sk_FragColor = vec4((_1_delta * src.w + src.x * (1.0 - dst.w)) + dst.x * (1.0 - src.w), (_3_delta * src.w + src.y * (1.0 - dst.w)) + dst.y * (1.0 - src.w), (_5_delta * src.w + src.z * (1.0 - dst.w)) + dst.z * (1.0 - src.w), src.w + (1.0 - src.w) * dst.w);
    float _6_dyTerm = dst.w == dst.x ? dst.w : 0.0;
    float _7_delta = abs(src.x) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, ((dst.w - dst.x) * src.w) / (src.x + sk_PrivkGuardedDivideEpsilon)) : _6_dyTerm;
    float _8_dyTerm = dst.w == dst.y ? dst.w : 0.0;
    float _9_delta = abs(src.y) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, ((dst.w - dst.y) * src.w) / (src.y + sk_PrivkGuardedDivideEpsilon)) : _8_dyTerm;
    float _10_dyTerm = dst.w == dst.z ? dst.w : 0.0;
    float _11_delta = abs(src.z) >= sk_PrivkMinNormalHalf ? dst.w - min(dst.w, ((dst.w - dst.z) * src.w) / (src.z + sk_PrivkGuardedDivideEpsilon)) : _10_dyTerm;
    sk_FragColor = vec4((_7_delta * src.w + src.x * (1.0 - dst.w)) + dst.x * (1.0 - src.w), (_9_delta * src.w + src.y * (1.0 - dst.w)) + dst.y * (1.0 - src.w), (_11_delta * src.w + src.z * (1.0 - dst.w)) + dst.z * (1.0 - src.w), src.w + (1.0 - src.w) * dst.w);
    sk_FragColor = dst.w == 0.0 ? src : vec4(soft_light_component_Qhh2h2(src.xw, dst.xw), soft_light_component_Qhh2h2(src.yw, dst.yw), soft_light_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
