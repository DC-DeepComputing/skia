#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
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
    half4 _0_result = half4(2.0h * _uniforms.dst.x <= _uniforms.dst.w ? (2.0h * _uniforms.src.x) * _uniforms.dst.x : _uniforms.src.w * _uniforms.dst.w - (2.0h * (_uniforms.dst.w - _uniforms.dst.x)) * (_uniforms.src.w - _uniforms.src.x), 2.0h * _uniforms.dst.y <= _uniforms.dst.w ? (2.0h * _uniforms.src.y) * _uniforms.dst.y : _uniforms.src.w * _uniforms.dst.w - (2.0h * (_uniforms.dst.w - _uniforms.dst.y)) * (_uniforms.src.w - _uniforms.src.y), 2.0h * _uniforms.dst.z <= _uniforms.dst.w ? (2.0h * _uniforms.src.z) * _uniforms.dst.z : _uniforms.src.w * _uniforms.dst.w - (2.0h * (_uniforms.dst.w - _uniforms.dst.z)) * (_uniforms.src.w - _uniforms.src.z), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    _0_result.xyz = _0_result.xyz + (_uniforms.dst.xyz * (1.0h - _uniforms.src.w) + _uniforms.src.xyz * (1.0h - _uniforms.dst.w));
    _out.sk_FragColor = _0_result;
    return _out;
}
