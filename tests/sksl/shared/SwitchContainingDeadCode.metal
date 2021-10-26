#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    int unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half value;
    switch (_uniforms.unknownInput) {
        case 0:
            value = 0.0h;
        case 1:
            value = 1.0h;
        default:
            value = 2.0h;
    }
    _out.sk_FragColor = half4(value);
    return _out;
}
