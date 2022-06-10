#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    int count;
    device int* a;
};
struct Outputs {
    device int* b;
};
kernel void computeMain(device int& count, device int* a, device int* b, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { count, a };
    Outputs _out = { b };
    int* _0_src = &_in.a[0];
    int* _1_dest = &_out.b[0];
    int _2_count = _in.count;
    while (_2_count > 0) {
        *_1_dest = *_0_src;
        ++_1_dest;
        ++_0_src;
        --_2_count;
    }
    array<int, 10> tmp;
    int* _3_src = &_in.a[0];
    int* _4_dest = &tmp[0];
    int _5_count = 10;
    while (_5_count > 0) {
        *_4_dest = *_3_src;
        ++_4_dest;
        ++_3_src;
        --_5_count;
    }
    return;
}
