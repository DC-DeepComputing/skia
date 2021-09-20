
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    {
        int _0_switchValue = int(colorGreen.y);
        bool _2_switchFallthrough = false;
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 0) {
                return colorRed;
            }
            if (_2_switchFallthrough || _0_switchValue == 1) {
                return colorGreen;
            }
            {
                return colorRed;
            }
        }
    }
}
