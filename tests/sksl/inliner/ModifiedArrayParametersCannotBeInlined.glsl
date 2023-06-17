
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float testArray[5];
float sumArrayMutating_ff(float a[5]) {
    for (int i = 1;i < 5; ++i) {
        a[0] += a[i];
    }
    return a[0];
}
vec4 main() {
    float _0_sum = 0.0;
    for (int _1_i = 0;_1_i < 5; ++_1_i) {
        _0_sum += testArray[_1_i];
    }
    return _0_sum == sumArrayMutating_ff(testArray) ? colorGreen : colorRed;
}
