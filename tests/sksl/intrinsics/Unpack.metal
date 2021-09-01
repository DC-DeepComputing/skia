#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    uint a;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.xy = half2(unpackHalf2x16(_uniforms.a));
    _out.sk_FragColor.xy = half2(unpackUnorm2x16(_uniforms.a));
    _out.sk_FragColor.xy = half2(unpackSnorm2x16(_uniforms.a));
    _out.sk_FragColor = half4(unpackUnorm4x8(_uniforms.a));
    _out.sk_FragColor = half4(unpackSnorm4x8(_uniforms.a));
    return _out;
}
