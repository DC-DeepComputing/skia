
out vec4 sk_FragColor;
uniform int value;
void main() {
    vec4 _4_result = vec4(1.0);
    {
        int _5_switchValue = value;
        bool _7_switchFallthrough = false;
        for (int _8_switchLoop = 0;_8_switchLoop < 1; _8_switchLoop++) {
            if (_5_switchValue == 0) {
                _4_result = vec4(0.5);
                _7_switchFallthrough = true;
            }
        }
    }
    sk_FragColor = _4_result;
}
