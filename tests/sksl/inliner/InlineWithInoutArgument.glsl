
out vec4 sk_FragColor;
uniform vec4 colorGreen;
void outParameterWrite_vh4(out vec4 x) {
    x = colorGreen;
}
void inoutParameterWrite_vh4(inout vec4 x) {
    x *= x;
}
vec4 main() {
    vec4 c;
    outParameterWrite_vh4(c);
    inoutParameterWrite_vh4(c);
    false;
    false;
    false;
    return c;
}
