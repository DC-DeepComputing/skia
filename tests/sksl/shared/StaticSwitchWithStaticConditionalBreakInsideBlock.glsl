
out vec4 sk_FragColor;
void main() {
    float x = 0.0;
    {
        int _0_switchValue = 0;
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                x = 0.0;
                if (x < 1.0) {
                    sk_FragColor = vec4(x);
                    break;
                }
                _2_switchFallthrough = true;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                x = 1.0;
                _2_switchFallthrough = true;
            }
        }
    }
}
