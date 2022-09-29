#include <metal_stdlib>
#include <simd/simd.h>
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
half4 blend_src_over_h4h4h4(half4 src, half4 dst);
half4 blend_src_over_h4h4h4(half4 src, half4 dst) {
    return src + (1.0h - src.w) * dst;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 _0_a = blend_src_over_h4h4h4(_uniforms.src, _uniforms.dst);
    half3 _1_b = (1.0h - _uniforms.dst.w) * _uniforms.src.xyz + _uniforms.dst.xyz;
    _0_a.xyz = min(_0_a.xyz, _1_b);
    _out.sk_FragColor = _0_a;
    return _out;
}
