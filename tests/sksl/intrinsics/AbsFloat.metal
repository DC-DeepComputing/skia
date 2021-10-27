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
    half4 expected = half4(1.25h, 0.0h, 0.75h, 2.25h);
    _out.sk_FragColor = ((((((abs(_uniforms.testInputs.x) == expected.x && all(abs(_uniforms.testInputs.xy) == expected.xy)) && all(abs(_uniforms.testInputs.xyz) == expected.xyz)) && all(abs(_uniforms.testInputs) == expected)) && 1.25h == expected.x) && all(half2(1.25h, 0.0h) == expected.xy)) && all(half3(1.25h, 0.0h, 0.75h) == expected.xyz)) && all(half4(1.25h, 0.0h, 0.75h, 2.25h) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
