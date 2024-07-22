
out vec4 sk_FragColor;
struct S {
    int y;
};
layout (binding = 0, set = 0) readonly buffer testStorageBuffer {
    float[] testArr;
};
layout (binding = 1, set = 0) readonly buffer testStorageBufferStruct {
    S[] testArrStruct;
};
int unsizedInParameterD_iS(S x[]) {
    return x[0].y;
}
vec4 getColor_h4f(float arr[]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
vec4 getColorSecond_h4f(float arr[]) {
    return vec4(arr[0], arr[1], arr[2], arr[3]);
}
void main() {
    sk_FragColor = getColor_h4f(testArr);
    sk_FragColor = getColorSecond_h4f(testArr);
    sk_FragColor = vec4(float(unsizedInParameterD_iS(testArrStruct)));
}
