#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct IndirectDispatchArgs {
    int x;
    int y;
    int z;
};
struct Inputs {
};
struct InterfaceBlock {
    IndirectDispatchArgs args;
} ib;
struct Globals {
    constant InterfaceBlock* ib;
};
kernel void computeMain(constant InterfaceBlock& ib [[buffer(0)]]) {
    Globals _globals{&ib};
    (void)_globals;
    Inputs _in = {  };
    return;
}
