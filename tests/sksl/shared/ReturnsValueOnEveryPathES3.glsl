
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool return_on_both_sides_b() {
    if (unknownInput == 1.0) return true; else return true;
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
bool for_with_double_sided_conditional_return_b() {
    for (int x = 0;x <= 10; ++x) {
        if (unknownInput == 1.0) return true; else return true;
    }
}
bool if_else_chain_b() {
    if (unknownInput == 1.0) return true; else if (unknownInput == 2.0) return false; else if (unknownInput == 3.0) return true; else if (unknownInput == 4.0) return false; else return true;
}
bool conditional_inside_while_loop_b() {
    while (unknownInput == 123.0) {
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
bool switch_with_all_returns_b() {
    {
        int _0_switchValue = int(unknownInput);
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
bool switch_fallthrough_b() {
    {
        int _8_switchValue = int(unknownInput);
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
bool switch_fallthrough_twice_b() {
    {
        int _12_switchValue = int(unknownInput);
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
bool switch_with_break_in_loop_b() {
    {
        int _16_switchValue = int(unknownInput);
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
bool switch_with_continue_in_loop_b() {
    {
        int _20_switchValue = int(unknownInput);
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
bool loop_with_continue_in_switch_b() {
    for (int x = 0;x <= 10; ++x) {
        {
            int _24_switchValue = int(unknownInput);
            bool _25_switchContinue = false;
            for (int _27_switchLoop = 0;_27_switchLoop < 1; _27_switchLoop++) {
                if (_24_switchValue == 1) {
                    {
                        _25_switchContinue = true;
                        break;
                    }
                }
                {
                    return true;
                }
            }
            if (_25_switchContinue) continue;
        }
    }
    return true;
}
bool switch_with_if_that_returns_b() {
    {
        int _28_switchValue = int(unknownInput);
        for (int _31_switchLoop = 0;_31_switchLoop < 1; _31_switchLoop++) {
            if (_28_switchValue == 1) {
                if (unknownInput == 123.0) return true; else return true;
            }
            {
                return true;
            }
        }
    }
}
bool switch_with_one_sided_if_then_fallthrough_b() {
    {
        int _32_switchValue = int(unknownInput);
        bool _34_switchFallthrough = false;
        for (int _35_switchLoop = 0;_35_switchLoop < 1; _35_switchLoop++) {
            if (_32_switchValue == 1) {
                if (unknownInput == 123.0) return true;
                _34_switchFallthrough = true;
            }
            {
                return true;
            }
        }
    }
}
vec4 main() {
    return ((((((((((((((((((true && return_on_both_sides_b()) && for_inside_body_b()) && after_for_body_b()) && for_with_double_sided_conditional_return_b()) && if_else_chain_b()) && conditional_inside_while_loop_b()) && inside_do_loop_b()) && inside_while_loop_b()) && after_do_loop_b()) && after_while_loop_b()) && switch_with_all_returns_b()) && switch_only_default_b()) && switch_fallthrough_b()) && switch_fallthrough_twice_b()) && switch_with_break_in_loop_b()) && loop_with_continue_in_switch_b()) && switch_with_continue_in_loop_b()) && switch_with_if_that_returns_b()) && switch_with_one_sided_if_then_fallthrough_b() ? colorGreen : colorRed;
}
