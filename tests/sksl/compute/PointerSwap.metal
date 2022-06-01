#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
};
kernel void computeMain(uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = {  };
    Outputs _out = {  };
int x = 1;
    int y = 2;
    int* _0_a = &x;
    int* _1_b = &y;
    int _2_temp = *_0_a;
    *_0_a = *_1_b;
    *_1_b = _2_temp;
}
