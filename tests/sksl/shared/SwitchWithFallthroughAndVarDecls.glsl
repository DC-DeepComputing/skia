#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok;
    {
        bool a;
        int b;
        int c;
        int _tmpSwitchValue1 = int(colorGreen.y), _tmpSwitchFallthrough0 = 0;
        for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
            if ((_tmpSwitchValue1 == 0)) {
                ;
                _tmpSwitchFallthrough0 = 1;
            }
            if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 1)) {
                b = 1;
                _tmpSwitchFallthrough0 = 1;
            }
            if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 2)) {
                {
                    float d = float(b);
                    c = int(d);
                }
                _tmpSwitchFallthrough0 = 1;
            }
            if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 3)) {
                a = bool(c);
                _tmpSwitchFallthrough0 = 1;
            }
            if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 4)) {
                ok = a;
                _tmpSwitchFallthrough0 = 1;
            }
        }
    }
    return ok ? colorGreen : colorRed;
}
