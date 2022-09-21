#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float f;
    array<float, 5> af;
    half4 h4;
    array<half4, 5> ah4;
};
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    half4 globalVar;
    S globalStruct;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _globals.globalVar = half4(0.0h);
    _globals.globalStruct.f = 0.0;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
