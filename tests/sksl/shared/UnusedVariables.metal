#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float a = 1.0;
    float b = 2.0;
    float c = 3.0;
    a = 77.0;
    b = 77.0;
    a = sin(c);
    b = sin(c);
    a = c + 1.0;
    b = c + 1.0;
    for (int x = 0;x < 1; ++x) {
        continue;
    }
    float d = c;
    b++;
    d++;
    _out.sk_FragColor = half4(half(b == 2.0), half(b == 3.0), half(d == 5.0), half(d == 4.0));
    return _out;
}
