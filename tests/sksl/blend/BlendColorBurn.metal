#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const half sk_PrivkGuardedDivideEpsilon = half(false ? 1e-08 : 0.0);
constant const half sk_PrivkMinNormalHalf = 6.10351562e-05h;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half guarded_divide_Qhhh(half n, half d);
half guarded_divide_Qhhh(half n, half d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_dyTerm = _uniforms.dst.w == _uniforms.dst.x ? _uniforms.dst.w : 0.0h;
    half _1_delta = abs(_uniforms.src.x) >= sk_PrivkMinNormalHalf ? _uniforms.dst.w - min(_uniforms.dst.w, guarded_divide_Qhhh((_uniforms.dst.w - _uniforms.dst.x) * _uniforms.src.w, _uniforms.src.x)) : _0_dyTerm;
    half _2_dyTerm = _uniforms.dst.w == _uniforms.dst.y ? _uniforms.dst.w : 0.0h;
    half _3_delta = abs(_uniforms.src.y) >= sk_PrivkMinNormalHalf ? _uniforms.dst.w - min(_uniforms.dst.w, guarded_divide_Qhhh((_uniforms.dst.w - _uniforms.dst.y) * _uniforms.src.w, _uniforms.src.y)) : _2_dyTerm;
    half _4_dyTerm = _uniforms.dst.w == _uniforms.dst.z ? _uniforms.dst.w : 0.0h;
    half _5_delta = abs(_uniforms.src.z) >= sk_PrivkMinNormalHalf ? _uniforms.dst.w - min(_uniforms.dst.w, guarded_divide_Qhhh((_uniforms.dst.w - _uniforms.dst.z) * _uniforms.src.w, _uniforms.src.z)) : _4_dyTerm;
    _out.sk_FragColor = half4((_1_delta * _uniforms.src.w + _uniforms.src.x * (1.0h - _uniforms.dst.w)) + _uniforms.dst.x * (1.0h - _uniforms.src.w), (_3_delta * _uniforms.src.w + _uniforms.src.y * (1.0h - _uniforms.dst.w)) + _uniforms.dst.y * (1.0h - _uniforms.src.w), (_5_delta * _uniforms.src.w + _uniforms.src.z * (1.0h - _uniforms.dst.w)) + _uniforms.dst.z * (1.0h - _uniforms.src.w), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}
