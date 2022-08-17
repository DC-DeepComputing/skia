#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    int zero = int(_uniforms.colorGreen.x);
    int one = int(_uniforms.colorGreen.y);
    for (int4 x = int4(zero);all((x < int4(100))); x += int4(one)) {
        for (int y = one;y < 100; y += one) {
            int4 _0_x = x;
            int4 _1_result = int4(0);
            while (all((_0_x >= int4(y)))) {
                _1_result += 1;
                _0_x -= int4(y);
            }
            if (any(x / y != _1_result)) {
                _out.sk_FragColor = half4(1.0h, half(float(x.x) / 255.0), half(float(y) / 255.0), 1.0h);
                return _out;
            }
        }
    }
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
