
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok;
    {
        bool a;
        int b;
        int c;
        switch (int(colorGreen.y)) {
            case 0:
            case 1:
                b = 1;
            case 2:
                {
                    float d = float(b);
                    c = int(d);
                }
            case 3:
                a = bool(c);
            case 4:
                ok = a;
        }
    }
    return ok ? colorGreen : colorRed;
}
