
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    vec4 expected = vec4(0.0);
    return (((((dFdx(testInputs.x) == expected.x && dFdx(testInputs.xy) == expected.xy) && dFdx(testInputs.xyz) == expected.xyz) && dFdx(testInputs) == expected) && sign(dFdx(sk_FragCoord.xx)) == vec2(1.0, 1.0)) && sign(dFdx(sk_FragCoord.yy)) == vec2(0.0, 0.0)) && sign(dFdx(sk_FragCoord)) == vec2(1.0, 0.0) ? colorGreen : colorRed;
}
