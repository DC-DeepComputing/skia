#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = 0.0;
    {
        int _0_switchValue = 0;
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                x = 0.0;
                if (x < 1.0) {
                    _out.sk_FragColor = float4(x);
                    break;
                }
                _2_switchFallthrough = true;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                x = 1.0;
                _2_switchFallthrough = true;
            }
        }
    }
    return _out;
}
