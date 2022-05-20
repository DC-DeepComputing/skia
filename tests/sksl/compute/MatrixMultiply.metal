#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    int2 size1;
    device float* data1;
    int2 size2;
    device float* data2;
};
struct Outputs {
    device int2& resultSize;
    device float* resultData;
};
kernel void computeMain(device int2& size1, device float* data1, device int2& size2, device float* data2, device int2& resultSize, device float* resultData, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Inputs _in = { sk_ThreadPosition, size1, data1, size2, data2 };
    Outputs _out = { resultSize, resultData };
_out.resultSize = int2(_in.size1.x, _in.size2.y);
    int2 resultCell = int2(int(_in.sk_ThreadPosition.x), int(_in.sk_ThreadPosition.y));
    float result = 0.0;
    for (int i = 0;i < _in.size1.y; ++i) {
        int a = i + resultCell.x * _in.size1.y;
        int b = resultCell.y + i * _in.size2.y;
        result += _in.data1[a] * _in.data2[b];
    }
    int index = resultCell.y + resultCell.x * _in.size2.y;
    _out.resultData[index] = result;
}
