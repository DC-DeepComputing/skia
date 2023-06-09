#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_twice_bi(int value) {
    bool ok = false;
    int _tmpSwitchValue1 = value, _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == 0)) {
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 1)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 2)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 3)) {
            ok = true;
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        break;
    }
    return ok;
}
bool switch_fallthrough_with_vardecls_bi(int value) {
    {
        bool a;
        bool b;
        int _tmpSwitchValue4 = value, _tmpSwitchFallthrough3 = 0;
        for (int _tmpSwitchLoop5 = 0; _tmpSwitchLoop5 < 1; _tmpSwitchLoop5++) {
            if ((_tmpSwitchValue4 == 0)) {
                ;
                _tmpSwitchFallthrough3 = 1;
            }
            if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 1)) {
                b = true;
                _tmpSwitchFallthrough3 = 1;
            }
            if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 2)) {
                a = b;
                _tmpSwitchFallthrough3 = 1;
            }
            if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 3)) {
                return a;
                _tmpSwitchFallthrough3 = 1;
            }
        }
    }
    return false;
}
vec4 main() {
    int x = int(colorGreen.y);
    bool _0_ok = false;
    int _tmpSwitchValue7 = x, _tmpSwitchFallthrough6 = 0;
    for (int _tmpSwitchLoop8 = 0; _tmpSwitchLoop8 < 1; _tmpSwitchLoop8++) {
        if ((_tmpSwitchValue7 == 2)) {
            break;
            _tmpSwitchFallthrough6 = 1;
        }
        if ((_tmpSwitchFallthrough6 > 0) || (_tmpSwitchValue7 == 1)) {
            ;
            _tmpSwitchFallthrough6 = 1;
        }
        if ((_tmpSwitchFallthrough6 > 0) || (_tmpSwitchValue7 == 0)) {
            _0_ok = true;
            break;
            _tmpSwitchFallthrough6 = 1;
        }
        break;
    }
    return (_0_ok && switch_fallthrough_twice_bi(x)) && switch_fallthrough_with_vardecls_bi(x) ? colorGreen : colorRed;
}
