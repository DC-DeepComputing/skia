
out vec4 sk_FragColor;
layout (binding = 0, set = 0) readonly buffer testStorageBuffer {
    float[] testArr;
};
vec4 getColor_h4f(float arr[-1]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
void main() {
    sk_FragColor = getColor_h4f(testArr);
}
