#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 input;
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
    half4 expectedVec = half4(1.0h, 0.0h, 0.0h, 0.0h);
    _out.sk_FragColor = ((((((sign(_uniforms.input.x) == expectedVec.x && all(normalize(_uniforms.input.xy) == expectedVec.xy)) && all(normalize(_uniforms.input.xyz) == expectedVec.xyz)) && all(normalize(_uniforms.input) == expectedVec)) && 1.0f == float(expectedVec.x)) && all(half2(0.0h, 1.0h) == expectedVec.yx)) && all(half3(0.0h, 1.0h, 0.0h) == expectedVec.zxy)) && all(half4(1.0h, 0.0h, 0.0h, 0.0h) == expectedVec) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
