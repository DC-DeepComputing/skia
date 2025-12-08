#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    int a;
    uint b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int _skTemp0;
    int _skTemp1;
    uint _skTemp2;
    int2 _skTemp3;
    uint2 _skTemp4;
    int3 _skTemp5;
    uint3 _skTemp6;
    int4 _skTemp7;
    uint4 _skTemp8;
    _out.sk_FragColor.x = half((_skTemp0 = (_uniforms.a), _skTemp1 = (select(_skTemp0, ~_skTemp0, _skTemp0 < 0)), select(int(clz(_skTemp1)), int(-1), _skTemp1 == int(0)))) + half((_skTemp2 = (_uniforms.b), select(int(clz(_skTemp2)), int(-1), _skTemp2 == uint(0))));
    _out.sk_FragColor.xy = half2((_skTemp3 = (int2(_uniforms.a)), select(int2(clz(_skTemp3)), int2(-1), _skTemp3 == int2(0)))) + half2((_skTemp4 = (uint2(_uniforms.b)), select(int2(clz(_skTemp4)), int2(-1), _skTemp4 == uint2(0))));
    _out.sk_FragColor.xyz = half3((_skTemp5 = (int3(_uniforms.a)), select(int3(clz(_skTemp5)), int3(-1), _skTemp5 == int3(0)))) + half3((_skTemp6 = (uint3(_uniforms.b)), select(int3(clz(_skTemp6)), int3(-1), _skTemp6 == uint3(0))));
    _out.sk_FragColor = half4((_skTemp7 = (int4(_uniforms.a)), select(int4(clz(_skTemp7)), int4(-1), _skTemp7 == int4(0)))) + half4((_skTemp8 = (uint4(_uniforms.b)), select(int4(clz(_skTemp8)), int4(-1), _skTemp8 == uint4(0))));
    return _out;
}
