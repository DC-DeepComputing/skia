#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
float2x2 float2x2_from_float3_float(float3 x0, float x1) {
    return float2x2(float2(x0.xy), float2(x0.z, x1));
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
float3x3 float3x3_from_float2_float2_float_float_float_float_float(float2 x0, float2 x1, float x2, float x3, float x4, float x5, float x6) {
    return float3x3(float3(x0.xy, x1.x), float3(x1.y, x2, x3), float3(x4, x5, x6));
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
float4x4 float4x4_from_float_float3_float3_float4_float2_float2_float(float x0, float3 x1, float3 x2, float4 x3, float2 x4, float2 x5, float x6) {
    return float4x4(float4(x0, x1.xyz), float4(x2.xyz, x3.x), float4(x3.yzw, x4.x), float4(x4.y, x5.xy, x6));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 f4 = float4_from_float2x2(_uniforms.testMatrix2x2);
    bool ok = float2x2_from_float3_float(f4.xyz, f4.w) == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    ok = ok && float3x3_from_float2_float2_float_float_float_float_float(f4.xy, f4.zw, f4.x, f4.y, f4.z, f4.w, f4.x) == float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 1.0, 2.0), float3(3.0, 4.0, 1.0));
    ok = ok && float4x4_from_float_float3_float3_float4_float2_float2_float(f4.x, f4.yzw, f4.xyz, f4.wxyz, f4.wx, f4.yz, f4.w) == float4x4(float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
