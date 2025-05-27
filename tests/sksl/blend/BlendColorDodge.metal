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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_dxScale = half(_uniforms.dst.x == 0.0h ? 0 : 1);
    half _1_delta = _0_dxScale * min(_uniforms.dst.w, abs(_uniforms.src.w - _uniforms.src.x) >= sk_PrivkMinNormalHalf ? (_uniforms.dst.x * _uniforms.src.w) / ((_uniforms.src.w - _uniforms.src.x) + sk_PrivkGuardedDivideEpsilon) : _uniforms.dst.w);
    half _2_dxScale = half(_uniforms.dst.y == 0.0h ? 0 : 1);
    half _3_delta = _2_dxScale * min(_uniforms.dst.w, abs(_uniforms.src.w - _uniforms.src.y) >= sk_PrivkMinNormalHalf ? (_uniforms.dst.y * _uniforms.src.w) / ((_uniforms.src.w - _uniforms.src.y) + sk_PrivkGuardedDivideEpsilon) : _uniforms.dst.w);
    half _4_dxScale = half(_uniforms.dst.z == 0.0h ? 0 : 1);
    half _5_delta = _4_dxScale * min(_uniforms.dst.w, abs(_uniforms.src.w - _uniforms.src.z) >= sk_PrivkMinNormalHalf ? (_uniforms.dst.z * _uniforms.src.w) / ((_uniforms.src.w - _uniforms.src.z) + sk_PrivkGuardedDivideEpsilon) : _uniforms.dst.w);
    _out.sk_FragColor = half4((_1_delta * _uniforms.src.w + _uniforms.src.x * (1.0h - _uniforms.dst.w)) + _uniforms.dst.x * (1.0h - _uniforms.src.w), (_3_delta * _uniforms.src.w + _uniforms.src.y * (1.0h - _uniforms.dst.w)) + _uniforms.dst.y * (1.0h - _uniforms.src.w), (_5_delta * _uniforms.src.w + _uniforms.src.z * (1.0h - _uniforms.dst.w)) + _uniforms.dst.z * (1.0h - _uniforms.src.w), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}
