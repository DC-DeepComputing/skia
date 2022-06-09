#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device int* a;
};
struct Outputs {
    device int* b;
};
kernel void computeMain(device int* a, device int* b, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, a };
    Outputs _out = { b };
    int* dest = &_out.b[0];
    dest += 5;
    dest -= 5;
    ++dest;
    --dest;
    int* test = &_out.b[5];
    if (test - dest == 5) {
        *(&_in.a[0] + _in.sk_ThreadPosition.x) = *(dest + _in.sk_ThreadPosition.x);
    }
    return;
}
