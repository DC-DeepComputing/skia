#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    {
        int _0_switchValue = int(_uniforms.colorGreen.y);
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                _out.sk_FragColor = _uniforms.colorRed;
                return _out;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                _out.sk_FragColor = _uniforms.colorGreen;
                return _out;
            }
            {
                _out.sk_FragColor = _uniforms.colorRed;
                return _out;
            }
        }
    }
    return _out;
}
