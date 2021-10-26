#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct sksl_synthetic_uniforms {
    float2 u_skRTFlip;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 expected = half4(0.0h);
    _out.sk_FragColor = (((((dfdx(_uniforms.testInputs.x) == expected.x && all(dfdx(_uniforms.testInputs.xy) == expected.xy)) && all(dfdx(_uniforms.testInputs.xyz) == expected.xyz)) && all(dfdx(_uniforms.testInputs) == expected)) && all(sign(_anonInterface0.u_skRTFlip.y*dfdy(coords.xx)) == float2(0.0f, 0.0f))) && all(sign(_anonInterface0.u_skRTFlip.y*dfdy(coords.yy)) == float2(1.0f, 1.0f))) && all(sign(_anonInterface0.u_skRTFlip.y*dfdy(coords)) == float2(0.0f, 1.0f)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
