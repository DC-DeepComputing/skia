
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
int numSideEffects = 0;
int side_effecting_ii(int value) {
    numSideEffects++;
    return value;
}
vec4 main() {
    const S _0_x = S(1, 2, 3);
    const S _2_y = S(1, 2, 4);
    int _6_two = 2;
    int _7_flatten0 = S(_0_x.a, _6_two, 3).a;
    int _8_flatten1 = S(_0_x.a, _6_two, 3).b;
    int _9_flatten2 = S(_0_x.a, _6_two, 3).c;
    int _10_val1 = 2;
    int _11_val2 = 1;
    int _12_noFlatten0 = S(--_10_val1, side_effecting_ii(2), 3).a;
    int _13_noFlatten1 = S(side_effecting_ii(1), 2, 3).b;
    int _14_noFlatten2 = S(1, _11_val2 += 1, 3).c;
    return ((((((((((_0_x != _2_y && !(_0_x == _2_y)) && _0_x.a == _2_y.a) && _7_flatten0 == _0_x.a) && _8_flatten1 == _0_x.b) && _9_flatten2 == _0_x.c) && _12_noFlatten0 == 1) && _13_noFlatten1 == 2) && _14_noFlatten2 == 3) && _10_val1 == 1) && _11_val2 == 2) && numSideEffects == 2 ? colorGreen : colorRed;
}
