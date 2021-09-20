
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _4_result;
    {
        int _5_switchValue = int(color.x);
        for (int _8_switchLoop = 0;_8_switchLoop < 1; _8_switchLoop++) {
            if (_5_switchValue == 1) {
                _4_result = color.yyyy;
                break;
            }
            {
                _4_result = color.zzzz;
                break;
            }
        }
    }
    sk_FragColor = _4_result;
}
