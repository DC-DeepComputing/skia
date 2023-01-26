#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    int mask;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    ok = ok && !(_uniforms.colorGreen.y == 1.0h);
    uint2 mask = uint2(uint(_uniforms.mask), ~uint(_uniforms.mask));
    int2 imask = int2(~mask);
    mask = ~mask & uint2(~imask);
    half4x4 m = half4x4(half(mask.x));
    _out.sk_FragColor = !(_uniforms.colorGreen.x == 1.0h) ? (-1.0 * m) * -_uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
