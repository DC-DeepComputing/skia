#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
    half4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float3x2 left, const float3x2 right);
thread bool operator!=(const float3x2 left, const float3x2 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);

thread bool operator==(const float4x2 left, const float4x2 right);
thread bool operator!=(const float4x2 left, const float4x2 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}

template <int C, int R>
matrix<float, C, R> outerProduct(const vec<float, R> a, const vec<float, C> b) {
    matrix<float, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a * b[c];
    }
    return result;
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
thread bool operator==(const float3x2 left, const float3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x2 left, const float3x2 right) {
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
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
    return !(left == right);
}
thread bool operator==(const float4x2 left, const float4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x2 left, const float4x2 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float2 c12 = float2(1.0f, 2.0f);
    _out.sk_FragColor = ((((outerProduct(_uniforms.testMatrix2x2[0], _uniforms.testMatrix2x2[1]) == float2x2(float2(3.0f, 6.0f), float2(4.0f, 8.0f)) && outerProduct(_uniforms.testMatrix3x3[0], _uniforms.testMatrix3x3[1]) == float3x3(float3(4.0f, 8.0f, 12.0f), float3(5.0f, 10.0f, 15.0f), float3(6.0f, 12.0f, 18.0f))) && outerProduct(_uniforms.testMatrix2x2[0], _uniforms.testMatrix3x3[1]) == float3x2(float2(4.0f, 8.0f), float2(5.0f, 10.0f), float2(6.0f, 12.0f))) && float4x4(outerProduct(_uniforms.testInputs, half4(1.0h, 0.0h, 0.0h, 2.0h))) == float4x4(float4(-1.25f, 0.0f, 0.75f, 2.25f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(-2.5f, 0.0f, 1.5f, 4.5f))) && outerProduct(float4(_uniforms.testInputs), c12) == float2x4(float4(-1.25f, 0.0f, 0.75f, 2.25f), float4(-2.5f, 0.0f, 1.5f, 4.5f))) && outerProduct(c12, float4(_uniforms.testInputs)) == float4x2(float2(-1.25f, -2.5f), float2(0.0f, 0.0f), float2(0.75f, 1.5f), float2(2.25f, 4.5f)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
