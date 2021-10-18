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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 expectedA = float4(-1.0f, 0.0f, 1.0f, 2.0f);
    _out.sk_FragColor = ((float(round(_uniforms.testInputs.x)) == -1.0f && all(float2(round(_uniforms.testInputs.xy)) == float2(-1.0f, 0.0f))) && all(float3(round(_uniforms.testInputs.xyz)) == float3(-1.0f, 0.0f, 1.0f))) && all(float4(round(_uniforms.testInputs)) == expectedA) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
