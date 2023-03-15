
out vec4 sk_FragColor;
struct SomeData {
    vec4 a;
    vec2 b;
};
layout (binding = 0, set = 0) readonly buffer storageBuffer {
    uint offset;
    SomeData[] inputData;
};
vec4 main() {
    return inputData[offset].a * inputData[offset].b.x;
}
