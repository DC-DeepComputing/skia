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
    float b = 2.0f;
    float c = 3.0f;
    for (int x = 0;x < 1; ++x) {
        continue;
    }
    float d = c;
    b++;
    d++;
    _out.sk_FragColor = half4(half(b == 2.0f), half(b == 3.0f), half(d == 5.0f), half(d == 4.0f));
    return _out;
}
