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
struct Globals {
    half3x3 a;
    half3x3 b;
    float4x4 c;
    float4x4 d;
};

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half4x4 left, const half4x4 right);
thread bool operator!=(const half4x4 left, const half4x4 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

template <int C, int R>
matrix<float, C, R> matrixCompMult(matrix<float, C, R> a, matrix<float, C, R> b) {
    matrix<float, C, R> result;
    for (int c = 0; c < C; ++c) {
        result[c] = a[c] * b[c];
    }
    return result;
}
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(const half4x4 left, const half4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x4 left, const half4x4 right) {
    return !(left == right);
}
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    half2x2 h22 = half2x2(half2(0.0h, 5.0h), half2(10.0h, 15.0h));
    half4x4 h44 = half4x4(half4(0.5h, 0.0h, 0.0h, 0.0h), half4(0.0h, 3.0h, 0.0h, 0.0h), half4(0.0h, 0.0h, 5.5h, 0.0h), half4(0.0h, 0.0h, 0.0h, 8.0h));
    float4x3 f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
    _out.sk_FragColor.xyz = matrixCompMult(_globals.a, _globals.b)[0];
    _out.sk_FragColor = half4(matrixCompMult(_globals.c, _globals.d)[0]);
    _out.sk_FragColor = (h22 == half2x2(half2(0.0h, 5.0h), half2(10.0h, 15.0h)) && h44 == half4x4(half4(0.5h, 0.0h, 0.0h, 0.0h), half4(0.0h, 3.0h, 0.0h, 0.0h), half4(0.0h, 0.0h, 5.5h, 0.0h), half4(0.0h, 0.0h, 0.0h, 8.0h))) && f43 == float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
