#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    int x;
};
struct Inputs {
    uint3 sk_ThreadPosition;
};
struct outputBuffer {
    int results[1];
};
struct Globals {
    device outputBuffer* _anonInterface0;
};
kernel void computeMain(constant Uniforms& _uniforms [[buffer(0)]], device outputBuffer& _anonInterface0 [[buffer(1)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition };
    _globals._anonInterface0->results[_in.sk_ThreadPosition.x] = _uniforms.x;
    return;
}
