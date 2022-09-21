
out vec4 sk_FragColor;
uniform vec4 colorGreen;
struct S {
    float f;
    float af[5];
    vec4 h4;
    vec4 ah4[5];
};
vec4 globalVar;
S globalStruct;
vec4 main() {
    globalVar = vec4(0.0);
    globalStruct.f = 0.0;
    return colorGreen;
}
