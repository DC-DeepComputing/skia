
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
const bool TRUE = true;
bool do_side_effect_bb(out bool x) {
    x = true;
    return false;
}
vec4 main() {
    bool ok;
    ok = (colorRed == colorGreen, true);
    ok = ok && (colorGreen.y == 1.0, true);
    ok = ok && (colorGreen.y == 0.0, TRUE);
    ok = ok || (colorGreen.y == 1.0, false);
    ok = ok || (colorGreen.y == 0.0, false);
    vec4 green = (coords.x == coords.y, colorGreen);
    vec4 red = (coords.x != coords.y, colorRed);
    bool param = false;
    bool call = (do_side_effect_bb(param), true);
    return (ok && param) && call ? green : red;
}
