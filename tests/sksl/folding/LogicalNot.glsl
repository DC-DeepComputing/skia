
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    ok = ok && colorGreen.x < colorGreen.y;
    ok = ok && colorGreen.x <= colorGreen.y;
    ok = ok && colorGreen.y > colorGreen.z;
    ok = ok && colorGreen.y >= colorGreen.z;
    ok = ok && colorGreen.y != colorGreen.x;
    ok = ok && colorGreen.y == colorGreen.w;
    return ok ? colorGreen : colorRed;
}
