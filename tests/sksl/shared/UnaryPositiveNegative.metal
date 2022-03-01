#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorWhite;
    half4 colorGreen;
    half4 colorRed;
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

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
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
bool test_iscalar_b(Uniforms _uniforms) {
    int x = int(_uniforms.colorWhite.x);
    x = -x;
    return x == -1;
}
bool test_fvec_b(Uniforms _uniforms) {
    half2 x = _uniforms.colorWhite.xy;
    x = -x;
    return all(x == half2(-1.0h));
}
bool test_ivec_b(Uniforms _uniforms) {
    int2 x = int2(int(_uniforms.colorWhite.x));
    x = -x;
    return all(x == int2(-1));
}
bool test_mat2_b(Uniforms _uniforms) {
    float2x2 x = float2x2(float(_uniforms.colorWhite.x));
    x = (-1.0 * x);
    return x == float2x2(-1.0);
}
bool test_mat3_b(Uniforms _uniforms) {
    float3x3 x = float3x3(float(_uniforms.colorWhite.x));
    x = (-1.0 * x);
    return x == float3x3(-1.0);
}
bool test_mat4_b(Uniforms _uniforms) {
    float4x4 x = float4x4(float(_uniforms.colorWhite.x));
    x = (-1.0 * x);
    return x == float4x4(-1.0);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _0_x = float(_uniforms.colorWhite.x);
    _0_x = -_0_x;
    _out.sk_FragColor = (((((_0_x == -1.0 && test_iscalar_b(_uniforms)) && test_fvec_b(_uniforms)) && test_ivec_b(_uniforms)) && test_mat2_b(_uniforms)) && test_mat3_b(_uniforms)) && test_mat4_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
