#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half2 h2;
    half3 h3;
    half4 h4;
    float2 f2;
    float3 f3;
    float4 f4;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(outerProduct(_uniforms.f2, _uniforms.f2)[1].xyyy);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f3, _uniforms.f3)[2].xyzz);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f4, _uniforms.f4)[3]);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f3, _uniforms.f2)[1].xyzz);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f2, _uniforms.f3)[2].xyyy);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f4, _uniforms.f2)[1]);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f2, _uniforms.f4)[3].xyyy);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f4, _uniforms.f3)[2]);
    _out.sk_FragColor = half4(outerProduct(_uniforms.f3, _uniforms.f4)[3].xyzz);
    _out.sk_FragColor = outerProduct(_uniforms.h2, _uniforms.h2)[1].xyyy;
    _out.sk_FragColor = outerProduct(_uniforms.h3, _uniforms.h3)[2].xyzz;
    _out.sk_FragColor = outerProduct(_uniforms.h4, _uniforms.h4)[3];
    _out.sk_FragColor = outerProduct(_uniforms.h3, _uniforms.h2)[1].xyzz;
    _out.sk_FragColor = outerProduct(_uniforms.h2, _uniforms.h3)[2].xyyy;
    _out.sk_FragColor = outerProduct(_uniforms.h4, _uniforms.h2)[1];
    _out.sk_FragColor = outerProduct(_uniforms.h2, _uniforms.h4)[3].xyyy;
    _out.sk_FragColor = outerProduct(_uniforms.h4, _uniforms.h3)[2];
    _out.sk_FragColor = outerProduct(_uniforms.h3, _uniforms.h4)[3].xyzz;
    return _out;
}
