#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 h22 = float2x2(float2(0.0, 5.0), float2(10.0, 15.0));
    float4x4 h44 = float4x4(float4(0.5, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0), float4(0.0, 0.0, 5.5, 0.0), float4(0.0, 0.0, 0.0, 8.0));
    _out.sk_FragColor = h22 == float2x2(float2(0.0, 5.0), float2(10.0, 15.0)) && h44 == float4x4(float4(0.5, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0), float4(0.0, 0.0, 5.5, 0.0), float4(0.0, 0.0, 0.0, 8.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
