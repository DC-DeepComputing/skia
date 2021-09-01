#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 input;
    half4 expected;
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
    const half4 constVal2 = half4(1.0h);
    _out.sk_FragColor = ((((((((((((((atan(_uniforms.input.x) == _uniforms.expected.x && all(atan(_uniforms.input.xy) == _uniforms.expected.xy)) && all(atan(_uniforms.input.xyz) == _uniforms.expected.xyz)) && all(atan(_uniforms.input) == _uniforms.expected)) && 0.0f == float(_uniforms.expected.x)) && all(half2(0.0h, 0.0h) == _uniforms.expected.xy)) && all(half3(0.0h, 0.0h, 0.0h) == _uniforms.expected.xyz)) && all(half4(0.0h, 0.0h, 0.0h, 0.0h) == _uniforms.expected)) && atan2(_uniforms.input.x, 1.0h) == _uniforms.expected.x) && all(atan2(_uniforms.input.xy, half2(1.0h)) == _uniforms.expected.xy)) && all(atan2(_uniforms.input.xyz, half3(1.0h)) == _uniforms.expected.xyz)) && all(atan2(_uniforms.input, constVal2) == _uniforms.expected)) && 0.0f == float(_uniforms.expected.x)) && all(half2(0.0h, 0.0h) == _uniforms.expected.xy)) && all(half3(0.0h, 0.0h, 0.0h) == _uniforms.expected.xyz)) && all(half4(0.0h, 0.0h, 0.0h, 0.0h) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
