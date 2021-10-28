
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(0.0);
    return (((((u_skRTFlip.y * dFdy(testInputs.x)) == expected.x && (u_skRTFlip.y * dFdy(testInputs.xy)) == expected.xy) && (u_skRTFlip.y * dFdy(testInputs.xyz)) == expected.xyz) && (u_skRTFlip.y * dFdy(testInputs)) == expected) && abs(sign((u_skRTFlip.y * dFdy(coords.x)))) == 0.0) && abs(sign((u_skRTFlip.y * dFdy(coords.y)))) == 1.0 ? colorGreen : colorRed;
}
