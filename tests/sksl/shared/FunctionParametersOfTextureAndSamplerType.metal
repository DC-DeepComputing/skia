#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

struct sampler2D {
    texture2d<half> tex;
    sampler smp;
};
half4 sample(sampler2D i, float2 p, float b=0) { return i.tex.sample(i.smp, p, bias(b)); }
half4 sample(sampler2D i, float3 p, float b=0) { return i.tex.sample(i.smp, p.xy / p.z, bias(b)); }
half4 sampleLod(sampler2D i, float2 p, float lod) { return i.tex.sample(i.smp, p, level(lod)); }
half4 sampleLod(sampler2D i, float3 p, float lod) {
    return i.tex.sample(i.smp, p.xy / p.z, level(lod));
}
half4 sampleGrad(sampler2D i, float2 p, float2 dPdx, float2 dPdy) {
    return i.tex.sample(i.smp, p, gradient2d(dPdx, dPdy));
}

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
    sampler2D cs;
};
half4 combined_sampler_only_helper_h4Z(Inputs _in, sampler2D cs_param) {
    return sample(cs_param, _in.c);
}
half4 bottom_helper_h4Tss(Inputs _in, texture2d<half> t_param, sampler s_param) {
    return sample(makeSampler2D(t_param, s_param), _in.c);
}
half4 helpers_helper_h4Tss(Inputs _in, texture2d<half> t_param, sampler s_param) {
    half4 color = sample(makeSampler2D(t_param, s_param), _in.c);
    return color.zyxw * bottom_helper_h4Tss(_in, t_param, s_param);
}
half4 helper_h4Tss(Inputs _in, texture2d<half> t_param, sampler s_param) {
    return helpers_helper_h4Tss(_in, t_param, s_param);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], texture2d<half> t [[texture(1)]], texture2d<half> cs_Tex [[texture(2)]], sampler cs_Smplr [[sampler(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{t, {cs_Tex, cs_Smplr}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = helper_h4Tss(_in, _uniforms.t, _uniforms.s) + combined_sampler_only_helper_h4Z(_in, _globals.cs);
    return _out;
}
