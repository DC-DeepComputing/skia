
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    int zero = int(colorGreen.x);
    int one = int(colorGreen.y);
    for (ivec4 x = ivec4(zero);all(lessThan(x, ivec4(100))); x += ivec4(one)) {
        for (int y = one;y < 100; y += one) {
            ivec4 _0_x = x;
            ivec4 _1_result = ivec4(0);
            while (all(greaterThanEqual(_0_x, ivec4(y)))) {
                _1_result += 1;
                _0_x -= ivec4(y);
            }
            if (x / y != _1_result) {
                return vec4(1.0, float(x.x) / 255.0, float(y) / 255.0, 1.0);
            }
        }
    }
    return colorGreen;
}
