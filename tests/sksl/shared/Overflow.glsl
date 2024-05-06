
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    float hugeH = ((1e+36 * 1e+36) * 1e+36) * 1e+27;
    float hugeF = ((1e+36 * 1e+36) * 1e+36) * 1e+27;
    int hugeI = 1073741824 * 1048576;
    uint hugeU = 2147483648u * 524288u;
    int hugeS = (16384 * 16384) * 8;
    uint hugeUS = (32768u * 32768u) * 2u;
    int hugeNI = -2147483648 * 524288;
    int hugeNS = (-32768 * 16384) * 4;
    ivec4 hugeIvec = ivec4(1073741824) * ivec4(32768);
    uvec4 hugeUvec = uvec4(2147483648u) * uvec4(16384u);
    mat4 hugeMxM = mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20) * mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    vec4 hugeMxV = mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20) * vec4(1e+20);
    vec4 hugeVxM = vec4(1e+20) * mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    return ((((((((((((colorGreen * clamp(hugeH, 0.0, 1.0)) * clamp(hugeF, 0.0, 1.0)) * clamp(float(hugeI), 0.0, 1.0)) * clamp(float(hugeU), 0.0, 1.0)) * clamp(float(hugeS), 0.0, 1.0)) * clamp(float(hugeUS), 0.0, 1.0)) * clamp(float(hugeNI), 0.0, 1.0)) * clamp(float(hugeNS), 0.0, 1.0)) * clamp(vec4(hugeIvec), 0.0, 1.0)) * clamp(vec4(hugeUvec), 0.0, 1.0)) * clamp(hugeMxM[0], 0.0, 1.0)) * clamp(hugeMxV, 0.0, 1.0)) * clamp(hugeVxM, 0.0, 1.0);
}
