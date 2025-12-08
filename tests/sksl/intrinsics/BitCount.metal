#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    int a;
    uint b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = half(popcount(_uniforms.a)) + half(popcount(_uniforms.b));
    _out.sk_FragColor.xy = half2(popcount(int2(_uniforms.a))) + half2(popcount(uint2(_uniforms.b)));
    _out.sk_FragColor.xyz = half3(popcount(int3(_uniforms.a))) + half3(popcount(uint3(_uniforms.b)));
    _out.sk_FragColor = half4(popcount(int4(_uniforms.a))) + half4(popcount(uint4(_uniforms.b)));
    return _out;
}
