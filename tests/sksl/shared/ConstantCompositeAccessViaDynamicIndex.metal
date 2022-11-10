#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
constant const array<half, 2> globalArray = array<half, 2>{1.0h, 1.0h};
constant const half2 globalVector = half2(1.0h, 1.0h);
constant const half2x2 globalMatrix = half2x2(half2(1.0h, 1.0h), half2(1.0h, 1.0h));
struct Uniforms {
    int zero;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const array<half, 2> localArray = array<half, 2>{0.0h, 1.0h};
    const half2 localVector = half2(1.0h, 1.0h);
    const half2x2 localMatrix = half2x2(half2(0.0h, 1.0h), half2(2.0h, 3.0h));
    _out.sk_FragColor = half4(globalArray[_uniforms.zero] * localArray[_uniforms.zero], globalVector[_uniforms.zero] * localVector[_uniforms.zero], globalMatrix[_uniforms.zero] * localMatrix[_uniforms.zero]);
    return _out;
}
