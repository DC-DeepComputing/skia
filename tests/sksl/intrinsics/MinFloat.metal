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
    half4 expectedA = half4(-1.25h, 0.0h, 0.5h, 0.5h);
    half4 expectedB = half4(-1.25h, 0.0h, 0.0h, 1.0h);
    _out.sk_FragColor = ((((((((((((((min(_uniforms.testInputs.x, 0.5h) == expectedA.x && all(min(_uniforms.testInputs.xy, 0.5h) == expectedA.xy)) && all(min(_uniforms.testInputs.xyz, 0.5h) == expectedA.xyz)) && all(min(_uniforms.testInputs, 0.5h) == expectedA)) && -1.25h == expectedA.x) && all(half2(-1.25h, 0.0h) == expectedA.xy)) && all(half3(-1.25h, 0.0h, 0.5h) == expectedA.xyz)) && all(half4(-1.25h, 0.0h, 0.5h, 0.5h) == expectedA)) && min(_uniforms.testInputs.x, _uniforms.colorGreen.x) == expectedB.x) && all(min(_uniforms.testInputs.xy, _uniforms.colorGreen.xy) == expectedB.xy)) && all(min(_uniforms.testInputs.xyz, _uniforms.colorGreen.xyz) == expectedB.xyz)) && all(min(_uniforms.testInputs, _uniforms.colorGreen) == expectedB)) && -1.25h == expectedB.x) && all(half2(-1.25h, 0.0h) == expectedB.xy)) && all(half3(-1.25h, 0.0h, 0.0h) == expectedB.xyz)) && all(half4(-1.25h, 0.0h, 0.0h, 1.0h) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
