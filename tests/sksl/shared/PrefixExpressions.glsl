
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform int mask;
vec4 main() {
    bool ok = true;
    ok = ok && !(colorGreen.y == 1.0);
    uvec2 mask = uvec2(uint(mask), ~uint(mask));
    ivec2 imask = ivec2(~mask);
    mask = ~mask & uvec2(~imask);
    mat4 m = mat4(float(mask.x));
    return !(colorGreen.x == 1.0) ? -m * -colorGreen : colorRed;
}
