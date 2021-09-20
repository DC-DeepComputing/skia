
out vec4 sk_FragColor;
uniform int unknownInput;
void main() {
    float value;
    {
        int _0_switchValue = unknownInput;
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                value = 0.0;
                _2_switchFallthrough = true;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                value = 1.0;
                _2_switchFallthrough = true;
            }
            {
                value = 2.0;
            }
        }
    }
    sk_FragColor = vec4(value);
}
