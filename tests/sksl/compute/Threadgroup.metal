#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
    device array<float, 512> in_data;
};
struct Outputs {
    device array<float, 512>& out_data;
};
struct Globals {
    threadgroup array<float, 1024> shared_data;
};
kernel void computeMain(device array<float, 512> in_data, device array<float, 512> out_data, uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{{}};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition, in_data };
    Outputs _out = { out_data };
    uint id = _in.sk_ThreadPosition.x;
    uint rd_id;
    uint wr_id;
    uint mask;
    _globals.shared_data[id * 2u] = _in.in_data[id * 2u];
    _globals.shared_data[id * 2u + 1u] = _in.in_data[id * 2u + 1u];
    threadgroupBarrier();
    const uint steps = 10u;
    for (uint step = 0u;step < steps; step++) {
        mask = (1u << step) - 1u;
        rd_id = ((id >> step) << step + 1u) + mask;
        wr_id = (rd_id + 1u) + (id & mask);
        _globals.shared_data[wr_id] += _globals.shared_data[rd_id];
        threadgroupBarrier();
    }
    _out.out_data[id * 2u] = _globals.shared_data[id * 2u];
    _out.out_data[id * 2u + 1u] = _globals.shared_data[id * 2u + 1u];
    return;
}
