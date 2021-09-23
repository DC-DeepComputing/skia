
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
vec4 main() {
    return (((((((((true && return_on_both_sides_b()) && for_inside_body_b()) && after_for_body_b()) && for_with_double_sided_conditional_return_b()) && if_else_chain_b()) && conditional_inside_while_loop_b()) && inside_do_loop_b()) && inside_while_loop_b()) && after_do_loop_b()) && after_while_loop_b() ? colorGreen : colorRed;
}
