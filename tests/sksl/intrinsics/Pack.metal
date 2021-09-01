#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half2 a;
    half4 b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = half(packHalf2x16(float2(_uniforms.a)));
    _out.sk_FragColor.x = half(packUnorm2x16(float2(_uniforms.a)));
    _out.sk_FragColor.x = half(packSnorm2x16(float2(_uniforms.a)));
    _out.sk_FragColor.x = half(packUnorm4x8(float4(_uniforms.b)));
    _out.sk_FragColor.x = half(packSnorm4x8(float4(_uniforms.b)));
    return _out;
}
