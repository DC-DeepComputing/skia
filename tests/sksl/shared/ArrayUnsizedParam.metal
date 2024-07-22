#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    int y;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct testStorageBuffer {
    float testArr[1];
};
struct testStorageBufferStruct {
    S testArrStruct[1];
};
struct Globals {
    const device testStorageBuffer* _anonInterface0;
    const device testStorageBufferStruct* _anonInterface1;
};
int unsizedInParameterD_iS(const device S* x) {
    return x[0].y;
}
half4 getColor_h4f(const device float* arr) {
    return half4(half(arr[0]), half(arr[1]), half(arr[2]), half(arr[3]));
}
half4 getColorSecond_h4f(const device float* arr) {
    return half4(half(arr[0]), half(arr[1]), half(arr[2]), half(arr[3]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device testStorageBuffer& _anonInterface0 [[buffer(0)]], const device testStorageBufferStruct& _anonInterface1 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = getColor_h4f(_globals._anonInterface0->testArr);
    _out.sk_FragColor = getColorSecond_h4f(_globals._anonInterface0->testArr);
    _out.sk_FragColor = half4(half(unsizedInParameterD_iS(_globals._anonInterface1->testArrStruct)));
    return _out;
}
