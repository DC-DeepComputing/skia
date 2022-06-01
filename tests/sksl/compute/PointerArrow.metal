#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Data {
    int* a;
    int* b;
};
struct Inputs {
    uint3 sk_ThreadPosition;
    device Data& data;
};
struct Outputs {
};
kernel void computeMain(device Data& data, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, data };
    Outputs _out = {  };
Data* _0_data = &_in.data;
    (*_0_data).a[_in.sk_ThreadPosition.x] = (*_0_data).b[_in.sk_ThreadPosition.x];
}
