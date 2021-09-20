
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    {
        int _0_switchValue = int(unknownInput);
        for (int _3_switchLoop = 0;_3_switchLoop < 1; _3_switchLoop++) {
            if (_0_switchValue == 1) {
                return vec4(1.0);
            }
            {
                return vec4(0.0);
            }
        }
    }
}
