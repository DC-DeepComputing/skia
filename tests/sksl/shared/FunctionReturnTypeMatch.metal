#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
thread bool operator==(const float2x2 left, const float2x2 right);thread bool operator!=(const float2x2 left, const float2x2 right);thread bool operator==(const float3x3 left, const float3x3 right);thread bool operator!=(const float3x3 left, const float3x3 right);thread bool operator==(const float4x4 left, const float4x4 right);thread bool operator!=(const float4x4 left, const float4x4 right);thread bool operator==(const half2x2 left, const half2x2 right);thread bool operator!=(const half2x2 left, const half2x2 right);thread bool operator==(const half3x3 left, const half3x3 right);thread bool operator!=(const half3x3 left, const half3x3 right);thread bool operator==(const half4x4 left, const half4x4 right);thread bool operator!=(const half4x4 left, const half4x4 right);thread bool operator==(const float2x2 left, const float2x2 right) {return all(left[0] == right[0]) && all(left[1] == right[1]);}thread bool operator!=(const float2x2 left, const float2x2 right) {return !(left == right);}thread bool operator==(const float3x3 left, const float3x3 right) {return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]);}thread bool operator!=(const float3x3 left, const float3x3 right) {return !(left == right);}thread bool operator==(const float4x4 left, const float4x4 right) {return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]) && all(left[3] == right[3]);}thread bool operator!=(const float4x4 left, const float4x4 right) {return !(left == right);}thread bool operator==(const half2x2 left, const half2x2 right) {return all(left[0] == right[0]) && all(left[1] == right[1]);}thread bool operator!=(const half2x2 left, const half2x2 right) {return !(left == right);}thread bool operator==(const half3x3 left, const half3x3 right) {return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]);}thread bool operator!=(const half3x3 left, const half3x3 right) {return !(left == right);}thread bool operator==(const half4x4 left, const half4x4 right) {return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]) && all(left[3] == right[3]);}thread bool operator!=(const half4x4 left, const half4x4 right) {return !(left == right);}fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x1 = 1.0;
    float2 x2 = float2(2.0);
    float3 x3 = float3(3.0);
    float4 x4 = float4(4.0);
    float2x2 x5 = float2x2(2.0);
    float3x3 x6 = float3x3(3.0);
    float4x4 x7 = float4x4(4.0);
    half x8 = 1.0h;
    half2 x9 = half2(2.0h);
    half3 x10 = half3(3.0h);
    half4 x11 = half4(4.0h);
    half2x2 x12 = half2x2(2.0h);
    half3x3 x13 = half3x3(3.0h);
    half4x4 x14 = half4x4(4.0h);
    bool x15 = true;
    bool2 x16 = bool2(true);
    bool3 x17 = bool3(true);
    bool4 x18 = bool4(true);
    int x19 = 1;
    int2 x20 = int2(2);
    int3 x21 = int3(3);
    int4 x22 = int4(4);
    _out.sk_FragColor = ((((((((((((((((((((x1 == 1.0 && all(x2 == float2(2.0))) && all(x3 == float3(3.0))) && all(x4 == float4(4.0))) && x5 == float2x2(2.0)) && x6 == float3x3(3.0)) && x7 == float4x4(4.0)) && x8 == 1.0h) && all(x9 == half2(2.0h))) && all(x10 == half3(3.0h))) && all(x11 == half4(4.0h))) && x12 == half2x2(2.0h)) && x13 == half3x3(3.0h)) && x14 == half4x4(4.0h)) && x15 == true) && all(x16 == bool2(true))) && all(x17 == bool3(true))) && all(x18 == bool4(true))) && x19 == 1) && all(x20 == int2(2))) && all(x21 == int3(3))) && all(x22 == int4(4)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
