#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
constant const int kConstant = 0;
constant const int kOtherConstant = 1;
constant const int kAnotherConstant = 2;
constant const float kFloatConstant = 2.14;
constant const float kFloatConstantAlias = kFloatConstant;
constant const half4 kConstVec = half4(1.0h, 0.2h, 2.14h, 1.0h);
struct Uniforms {
    int integerInput;
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float kLocalFloatConstant = 3.14;
    const float kLocalFloatConstantAlias = kLocalFloatConstant;
    if (_uniforms.integerInput == kConstant) {
        _out.sk_FragColor = half4(2.14h);
        return _out;
    } else if (_uniforms.integerInput == kOtherConstant) {
        _out.sk_FragColor = _uniforms.colorGreen;
        return _out;
    } else if (_uniforms.integerInput == kAnotherConstant) {
        _out.sk_FragColor = kConstVec;
        return _out;
    } else if (kLocalFloatConstantAlias < float(_uniforms.colorGreen.x) * kLocalFloatConstant) {
        _out.sk_FragColor = half4(3.14h);
        return _out;
    } else if (kFloatConstantAlias >= float(_uniforms.colorGreen.x) * kFloatConstantAlias) {
        _out.sk_FragColor = half4(0.0h);
        return _out;
    } else {
        _out.sk_FragColor = half4(1.0h, 0.0h, 0.0h, 1.0h);
        return _out;
    }
    return _out;
}
