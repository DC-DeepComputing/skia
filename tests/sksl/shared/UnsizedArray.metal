#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct ElementWithArrayMember {
    array<float, 2> array;
};
struct ElementWithNoArray {
    float2 a;
};
struct Inputs {
    uint3 sk_GlobalInvocationID;
};
struct inputElements {
    ElementWithArrayMember wideStrideElementArray[1];
};
struct unsizedArrayAlignmentTest {
    uint foo;
    float factors[1];
};
struct outputElementsBuffer {
    ElementWithArrayMember outputElements[1];
};
struct packedElementsBuffer {
    ElementWithNoArray packedElements[1];
};
struct Globals {
    const device inputElements* _anonInterface0;
    const device unsizedArrayAlignmentTest* _anonInterface1;
    device outputElementsBuffer* _anonInterface2;
    device packedElementsBuffer* _anonInterface3;
};
kernel void computeMain(uint3 sk_GlobalInvocationID [[thread_position_in_grid]], const device inputElements& _anonInterface0 [[buffer(0)]], const device unsizedArrayAlignmentTest& _anonInterface1 [[buffer(1)]], device outputElementsBuffer& _anonInterface2 [[buffer(2)]], device packedElementsBuffer& _anonInterface3 [[buffer(3)]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1, &_anonInterface2, &_anonInterface3};
    (void)_globals;
    Inputs _in = { sk_GlobalInvocationID };
    uint idx = _in.sk_GlobalInvocationID.x;
    ElementWithArrayMember _0_e = _globals._anonInterface0->wideStrideElementArray[idx];
    float f = _0_e.array[0] * _0_e.array[1];
    array<float, 2> _1_arr = _globals._anonInterface0->wideStrideElementArray[idx].array;
    f *= _1_arr[0] * _1_arr[1];
    ElementWithArrayMember e;
    e.array[0] = f * _globals._anonInterface1->factors[idx];
    e.array[1] = f * _globals._anonInterface1->factors[idx + 1u];
    _globals._anonInterface2->outputElements[idx] = e;
    return;
}
