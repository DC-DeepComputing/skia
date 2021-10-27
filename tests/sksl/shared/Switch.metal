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
    half4 color;
    switch (int(_uniforms.colorGreen.y)) {
        case 0:
            color = _uniforms.colorRed;
            break;
        case 1:
            color = _uniforms.colorGreen;
            break;
        default:
            color = _uniforms.colorRed;
            break;
    }
    _out.sk_FragColor = color;
    return _out;
}
