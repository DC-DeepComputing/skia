#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
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
    float4 expected = float4(0.0);
    _out.sk_FragColor = ((((dfdx(_uniforms.testInputs.x) == expected.x && all(dfdx(_uniforms.testInputs.xy) == expected.xy)) && all(dfdx(_uniforms.testInputs.xyz) == expected.xyz)) && all(dfdx(_uniforms.testInputs) == expected)) && abs(sign(dfdx(coords.x))) == 1.0) && abs(sign(dfdx(coords.y))) == 0.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
