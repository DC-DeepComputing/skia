#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float value = 0.0;
    {
        int _0_switchValue = int(_uniforms.unknownInput);
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                value = 0.0;
                _2_switchFallthrough = true;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                value = 1.0;
                _2_switchFallthrough = true;
            }
        }
    }
    _out.sk_FragColor = float4(value);
    return _out;
}
