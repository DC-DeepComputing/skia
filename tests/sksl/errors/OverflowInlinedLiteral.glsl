
int cast_int_to_short_si(int x) {
    return x;
}
ivec2 cast_int2_to_short2_s2i2(ivec2 x) {
    return x;
}
int cast_float_to_int_if(float x) {
    return int(x);
}
ivec3 cast_float3_to_int3_i3f3(vec3 x) {
    return ivec3(x);
}
int negate_short_ss(int x) {
    return -x;
}
ivec4 negate_int4_i4i4(ivec4 x) {
    return -x;
}
void main() {
    cast_int_to_short_si(99999);
    cast_int2_to_short2_s2i2(ivec2(12345, 67890));
    cast_float_to_int_if(5000000000.0);
    cast_float3_to_int3_i3f3(vec3(3000000000.0, 2000000.0, 1000.0));
    negate_short_ss(-32768);
    negate_int4_i4i4(ivec4(-2147483648));
}
