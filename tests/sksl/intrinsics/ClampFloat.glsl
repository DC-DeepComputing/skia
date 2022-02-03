
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedA = vec4(-1.0, 0.0, 0.75, 1.0);
    const vec4 clampLow = vec4(-1.0, -2.0, -2.0, 1.0);
    vec4 expectedB = vec4(-1.0, 0.0, 0.5, 2.25);
    const vec4 clampHigh = vec4(1.0, 2.0, 0.5, 3.0);
    return ((((((((((((((distance(clamp(testInputs.x, -1.0, 1.0), expectedA.x) <= 0.0010000000474974513 && distance(clamp(testInputs.xy, -1.0, 1.0), expectedA.xy) <= 0.0010000000474974513) && distance(clamp(testInputs.xyz, -1.0, 1.0), expectedA.xyz) <= 0.0010000000474974513) && distance(clamp(testInputs, -1.0, 1.0), expectedA) <= 0.0010000000474974513) && distance(clamp(testInputs.x, -1.0, 1.0), expectedB.x) <= 0.0010000000474974513) && distance(clamp(testInputs.xy, vec2(-1.0, -2.0), vec2(1.0, 2.0)), expectedB.xy) <= 0.0010000000474974513) && distance(clamp(testInputs.xyz, vec3(-1.0, -2.0, -2.0), vec3(1.0, 2.0, 0.5)), expectedB.xyz) <= 0.0010000000474974513) && distance(clamp(testInputs, clampLow, clampHigh), expectedB) <= 0.0010000000474974513) && -1.0 == expectedA.x) && vec2(-1.0, 0.0) == expectedA.xy) && vec3(-1.0, 0.0, 0.75) == expectedA.xyz) && vec4(-1.0, 0.0, 0.75, 1.0) == expectedA) && -1.0 == expectedB.x) && vec2(-1.0, 0.0) == expectedB.xy) && vec3(-1.0, 0.0, 0.5) == expectedB.xyz) && vec4(-1.0, 0.0, 0.5, 2.25) == expectedB ? colorGreen : colorRed;
}
