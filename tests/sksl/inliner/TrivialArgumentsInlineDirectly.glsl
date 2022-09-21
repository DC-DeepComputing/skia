
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
uniform mat2 testMatrix2x2;
struct S {
    vec4 ah4[1];
    float ah[1];
    vec4 h4;
    float h;
};
struct S4 {
    float a;
    float b;
    float c;
    float d;
};
struct S5 {
    float a;
    float b;
    float c;
    float d;
    float e;
};
vec4 main() {
    S s;
    s.ah4[0] = vec4(unknownInput);
    s.ah[0] = unknownInput;
    s.h4 = vec4(unknownInput);
    s.h = unknownInput;
    bool b = bool(unknownInput);
    return colorGreen;
}
