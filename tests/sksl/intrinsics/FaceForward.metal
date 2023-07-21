#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    float huge = float(((((1e+30h) * (1e+30h) < 0) ? 1 : -1) * (1.0h)));
    float2 huge2 = faceforward(float2(1.0), float2(1e+30), float2(1e+30));
    float3 huge3 = faceforward(float3(1.0), float3(1e+30), float3(1e+30));
    float4 huge4 = faceforward(float4(1.0), float4(1e+30), float4(1e+30));
    half4 expectedPos = half4(float4(huge) + huge2.xxxx);
    half4 expectedNeg = half4(huge3.xxxx + huge4.xxxx);
    const half4 constN = half4(1.0h, 2.0h, 3.0h, 4.0h);
    const half4 constI = half4(1.0h, 1.0h, -100.0h, 1.0h);
    half4 N = _uniforms.colorGreen.yyyy * constN;
    half4 I = _uniforms.colorGreen.yyyy * constI;
    half4 NRef = _uniforms.colorGreen.yyyy;
    expectedPos = half4(1.0h, 2.0h, 3.0h, 4.0h);
    expectedNeg = half4(-1.0h, -2.0h, -3.0h, -4.0h);
    _out.sk_FragColor = ((((((((((NRef.x) * (I.x) < 0) ? 1 : -1) * (N.x)) == expectedNeg.x && all(faceforward(N.xy, I.xy, NRef.xy) == expectedNeg.xy)) && all(faceforward(N.xyz, I.xyz, NRef.xyz) == expectedPos.xyz)) && all(faceforward(N, I, NRef) == expectedPos)) && -1.0h == expectedNeg.x) && all(half2(-1.0h, -2.0h) == expectedNeg.xy)) && all(half3(1.0h, 2.0h, 3.0h) == expectedPos.xyz)) && all(half4(1.0h, 2.0h, 3.0h, 4.0h) == expectedPos) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
