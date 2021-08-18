#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct sksl_synthetic_uniforms {
    float2 u_skRTFlip;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 expected = float4(0.0);
    _out.sk_FragColor = (((((dfdx(_uniforms.testInputs.x) == expected.x && all(dfdx(_uniforms.testInputs.xy) == expected.xy)) && all(dfdx(_uniforms.testInputs.xyz) == expected.xyz)) && all(dfdx(_uniforms.testInputs) == expected)) && all(sign(dfdx(float4(_fragCoord.x, _anonInterface0.u_skRTFlip.x + _anonInterface0.u_skRTFlip.y * _fragCoord.y, 0.0, _fragCoord.w).xx)) == float2(1.0, 1.0))) && all(sign(dfdx(float4(_fragCoord.x, _anonInterface0.u_skRTFlip.x + _anonInterface0.u_skRTFlip.y * _fragCoord.y, 0.0, _fragCoord.w).yy)) == float2(0.0, 0.0))) && all(sign(dfdx(float4(_fragCoord.x, _anonInterface0.u_skRTFlip.x + _anonInterface0.u_skRTFlip.y * _fragCoord.y, 0.0, _fragCoord.w))) == float2(1.0, 0.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
