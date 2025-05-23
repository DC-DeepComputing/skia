#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half4 blend_overlay_h4h4h4(half4 src, half4 dst);
half4 blend_overlay_h4h4h4(half4 src, half4 dst) {
    half4 result = half4(2.0h * dst.x <= dst.w ? (2.0h * src.x) * dst.x : src.w * dst.w - (2.0h * (dst.w - dst.x)) * (src.w - src.x), 2.0h * dst.y <= dst.w ? (2.0h * src.y) * dst.y : src.w * dst.w - (2.0h * (dst.w - dst.y)) * (src.w - src.y), 2.0h * dst.z <= dst.w ? (2.0h * src.z) * dst.z : src.w * dst.w - (2.0h * (dst.w - dst.z)) * (src.w - src.z), src.w + (1.0h - src.w) * dst.w);
    result.xyz = result.xyz + (dst.xyz * (1.0h - src.w) + src.xyz * (1.0h - dst.w));
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_overlay_h4h4h4(_uniforms.dst, _uniforms.src);
    return _out;
}
