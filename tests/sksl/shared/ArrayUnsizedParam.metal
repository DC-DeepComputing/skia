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
struct Globals {
    const device testStorageBuffer* _anonInterface0;
};
half4 getColor_h4f(const device float* arr) {
    return half4(half(arr[0]), half(arr[1]), half(arr[2]), half(arr[3]));
}
half4 getColorSecond_h4f(const device float* arr) {
    return half4(half(arr[0]), half(arr[1]), half(arr[2]), half(arr[3]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], const device testStorageBuffer& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = getColor_h4f(_globals._anonInterface0->testArr);
    _out.sk_FragColor = getColorSecond_h4f(_globals._anonInterface0->testArr);
    return _out;
}
