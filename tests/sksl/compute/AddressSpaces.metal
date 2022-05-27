#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    constant int* src1;
    thread int* src2;
};
struct Outputs {
    threadgroup int* dest;
    device int* deviceVar;
};
kernel void computeMain(constant int* src1, thread int* src2, threadgroup int* dest, device int* deviceVar, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, src1, src2 };
    Outputs _out = { dest, deviceVar };
_out.dest[_in.sk_ThreadPosition.x] = _in.src1[_in.sk_ThreadPosition.x] + _in.src2[_in.sk_ThreadPosition.x];
}
