
struct ElementWithArrayMember {
    float array[2];
};
struct ElementWithNoArray {
    vec2 a;
};
layout (binding = 0, set = 0) readonly buffer inputElements {
    ElementWithArrayMember[] wideStrideElementArray;
};
layout (binding = 1, set = 0) readonly buffer unsizedArrayAlignmentTest {
    uint foo;
    float[] factors;
};
layout (binding = 2, set = 0) buffer outputElementsBuffer {
    ElementWithArrayMember[] outputElements;
};
layout (binding = 3, set = 0) buffer packedElementsBuffer {
    ElementWithNoArray[] packedElements;
};
void main() {
    uint idx = sk_GlobalInvocationID.x;
    ElementWithArrayMember _0_e = wideStrideElementArray[idx];
    float f = _0_e.array[0] * _0_e.array[1];
    float _1_arr[2] = wideStrideElementArray[idx].array;
    f *= _1_arr[0] * _1_arr[1];
    ElementWithArrayMember e;
    e.array[0] = f * factors[idx];
    e.array[1] = f * factors[idx + 1u];
    outputElements[idx] = e;
}
