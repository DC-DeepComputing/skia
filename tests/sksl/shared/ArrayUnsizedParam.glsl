
out vec4 sk_FragColor;
layout (binding = 0, set = 0) readonly buffer testStorageBuffer {
    float[] testArr;
};
vec4 anonymousParam_h4f(float _skAnonymousParam0[-1]);
vec4 getColor_h4f(float arr[-1]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
vec4 getColor_helper_h4f(float arr[-1]) {
    return getColor_h4f(arr);
}
vec4 getColorSecond_h4f(float arr[-1]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
void main() {
    sk_FragColor = getColor_helper_h4f(testArr);
    sk_FragColor = getColorSecond_h4f(testArr);
}
