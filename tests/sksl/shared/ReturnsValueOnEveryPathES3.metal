#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
bool return_on_both_sides_b(Uniforms _uniforms) {
    if (_uniforms.unknownInput == 1.0) return true; else return true;
}
bool for_inside_body_b() {
    for (int x = 0;x <= 10; ++x) {
        return true;
    }
}
bool after_for_body_b() {
    for (int x = 0;x <= 10; ++x) {
        true;
    }
    return true;
}
bool for_with_double_sided_conditional_return_b(Uniforms _uniforms) {
    for (int x = 0;x <= 10; ++x) {
        if (_uniforms.unknownInput == 1.0) return true; else return true;
    }
}
bool if_else_chain_b(Uniforms _uniforms) {
    if (_uniforms.unknownInput == 1.0) return true; else if (_uniforms.unknownInput == 2.0) return false; else if (_uniforms.unknownInput == 3.0) return true; else if (_uniforms.unknownInput == 4.0) return false; else return true;
}
bool conditional_inside_while_loop_b(Uniforms _uniforms) {
    while (_uniforms.unknownInput == 123.0) {
        return true;
    }
}
bool inside_do_loop_b() {
    do {
        return true;
    } while (true);
}
bool inside_while_loop_b() {
    while (true) {
        return true;
    }
}
bool after_do_loop_b() {
    do {
        break;
    } while (true);
    return true;
}
bool after_while_loop_b() {
    while (true) {
        break;
    }
    return true;
}
bool switch_with_all_returns_b(Uniforms _uniforms) {
    {
        int _0_switchValue = int(_uniforms.unknownInput);
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 1) {
                return true;
            }
            if (_2_switchFallthrough || _0_switchValue == 2) {
                return true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_only_default_b() {
    {
        for (int _7_switchLoop = 0;_7_switchLoop < 1; _7_switchLoop++) {
            {
                return true;
            }
        }
    }
}
bool switch_fallthrough_b(Uniforms _uniforms) {
    {
        int _8_switchValue = int(_uniforms.unknownInput);
        bool _10_switchFallthrough = false;
        for (int _11_switchLoop = 0;_11_switchLoop < 1; _11_switchLoop++) {
            if (_8_switchValue == 1) {
                return true;
            }
            if (_10_switchFallthrough || _8_switchValue == 2) {
                _10_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_fallthrough_twice_b(Uniforms _uniforms) {
    {
        int _12_switchValue = int(_uniforms.unknownInput);
        bool _14_switchFallthrough = false;
        for (int _15_switchLoop = 0;_15_switchLoop < 1; _15_switchLoop++) {
            if (_12_switchValue == 1) {
                _14_switchFallthrough = true;
            }
            if (_14_switchFallthrough || _12_switchValue == 2) {
                _14_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_with_break_in_loop_b(Uniforms _uniforms) {
    {
        int _16_switchValue = int(_uniforms.unknownInput);
        bool _18_switchFallthrough = false;
        for (int _19_switchLoop = 0;_19_switchLoop < 1; _19_switchLoop++) {
            if (_16_switchValue == 1) {
                for (int x = 0;x <= 10; ++x) {
                    break;
                }
                _18_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_with_continue_in_loop_b(Uniforms _uniforms) {
    {
        int _20_switchValue = int(_uniforms.unknownInput);
        bool _22_switchFallthrough = false;
        for (int _23_switchLoop = 0;_23_switchLoop < 1; _23_switchLoop++) {
            if (_20_switchValue == 1) {
                for (int x = 0;x <= 10; ++x) {
                    continue;
                }
                _22_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_with_if_that_returns_b(Uniforms _uniforms) {
    {
        int _24_switchValue = int(_uniforms.unknownInput);
        for (int _27_switchLoop = 0;_27_switchLoop < 1; _27_switchLoop++) {
            if (_24_switchValue == 1) {
                if (_uniforms.unknownInput == 123.0) return true; else return true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_with_one_sided_if_then_fallthrough_b(Uniforms _uniforms) {
    {
        int _28_switchValue = int(_uniforms.unknownInput);
        bool _30_switchFallthrough = false;
        for (int _31_switchLoop = 0;_31_switchLoop < 1; _31_switchLoop++) {
            if (_28_switchValue == 1) {
                if (_uniforms.unknownInput == 123.0) return true;
                _30_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = (((((((((((((((((true && return_on_both_sides_b(_uniforms)) && for_inside_body_b()) && after_for_body_b()) && for_with_double_sided_conditional_return_b(_uniforms)) && if_else_chain_b(_uniforms)) && conditional_inside_while_loop_b(_uniforms)) && inside_do_loop_b()) && inside_while_loop_b()) && after_do_loop_b()) && after_while_loop_b()) && switch_with_all_returns_b(_uniforms)) && switch_only_default_b()) && switch_fallthrough_b(_uniforms)) && switch_fallthrough_twice_b(_uniforms)) && switch_with_break_in_loop_b(_uniforms)) && switch_with_continue_in_loop_b(_uniforms)) && switch_with_if_that_returns_b(_uniforms)) && switch_with_one_sided_if_then_fallthrough_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
