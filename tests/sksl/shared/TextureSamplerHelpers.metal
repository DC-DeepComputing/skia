#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    sampler s;
};
struct Inputs {
    float2 c  [[user(locn1)]];
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    texture2d<half> t;
};
half4 doTextureStuff_h4ssT(Inputs _in, sampler s, texture2d<half> t) {
    return sample(makeSampler2D(t, s), _in.c).zyxw;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], texture2d<half> t [[texture(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{t};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = doTextureStuff_h4ssT(_in, _uniforms.s, _uniforms.t);
    return _out;
}
