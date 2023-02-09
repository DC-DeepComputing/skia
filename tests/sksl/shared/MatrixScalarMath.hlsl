cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
    float4 _12_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _31, float _32, float _33, float _34, float _35, float2x2 _36)
{
    float one = _12_colorRed.x;
    float2 _53 = float2(_32 * _12_colorRed.x, _33 * _12_colorRed.x);
    float2 _54 = float2(_34 * _12_colorRed.x, _35 * _12_colorRed.x);
    float2x2 m2 = float2x2(_53, _54);
    switch (_31)
    {
        case 1:
        {
            m2 = float2x2(_53 + 1.0f.xx, _54 + 1.0f.xx);
            break;
        }
        case 2:
        {
            m2 = float2x2(m2[0] - 1.0f.xx, m2[1] - 1.0f.xx);
            break;
        }
        case 3:
        {
            m2 = m2 * 2.0f;
            break;
        }
        case 4:
        {
            m2 = m2 * 0.5f;
            break;
        }
    }
    bool _98 = false;
    if (m2[0].x == _36[0].x)
    {
        _98 = m2[0].y == _36[0].y;
    }
    else
    {
        _98 = false;
    }
    bool _108 = false;
    if (_98)
    {
        _108 = m2[1].x == _36[1].x;
    }
    else
    {
        _108 = false;
    }
    bool _118 = false;
    if (_108)
    {
        _118 = m2[1].y == _36[1].y;
    }
    else
    {
        _118 = false;
    }
    return _118;
}

bool divisionTest_b()
{
    float _126 = _12_colorRed.x * 10.0f;
    float ten = _126;
    float2 _128 = _126.xx;
    float2 _134 = _12_testInputs.x.xx;
    float2 _136 = _128 / _134;
    float2 _137 = _128 / _134;
    float2x2 div = float2x2(_136, _137);
    return all(bool2(_136.x == (-8.0f).xx.x, _136.y == (-8.0f).xx.y)) && all(bool2(_137.x == (-8.0f).xx.x, _137.y == (-8.0f).xx.y));
}

float4 main(float2 _149)
{
    float f1 = _12_colorGreen.y;
    float _159 = 2.0f * _12_colorGreen.y;
    float f2 = _159;
    float _165 = 3.0f * _12_colorGreen.y;
    float f3 = _165;
    float _171 = 4.0f * _12_colorGreen.y;
    float f4 = _171;
    float2x2 _RESERVED_IDENTIFIER_FIXUP_0_expected = float2x2(float2(_12_colorGreen.y + 1.0f, _159 + 1.0f), float2(_165 + 1.0f, _171 + 1.0f));
    float _RESERVED_IDENTIFIER_FIXUP_1_one = _12_colorRed.x;
    float2 _189 = float2(_12_colorGreen.y * _12_colorRed.x, _159 * _12_colorRed.x);
    float2 _190 = float2(_165 * _12_colorRed.x, _171 * _12_colorRed.x);
    float2x2 _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_189, _190);
    _RESERVED_IDENTIFIER_FIXUP_2_m2 = float2x2(_189 + 1.0f.xx, _190 + 1.0f.xx);
    bool _211 = false;
    if (_RESERVED_IDENTIFIER_FIXUP_2_m2[0].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].x)
    {
        _211 = _RESERVED_IDENTIFIER_FIXUP_2_m2[0].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[0].y;
    }
    else
    {
        _211 = false;
    }
    bool _221 = false;
    if (_211)
    {
        _221 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].x == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].x;
    }
    else
    {
        _221 = false;
    }
    bool _231 = false;
    if (_221)
    {
        _231 = _RESERVED_IDENTIFIER_FIXUP_2_m2[1].y == _RESERVED_IDENTIFIER_FIXUP_0_expected[1].y;
    }
    else
    {
        _231 = false;
    }
    bool _248 = false;
    if (_231)
    {
        int _234 = 2;
        float _235 = _12_colorGreen.y;
        float _236 = _159;
        float _237 = _165;
        float _238 = _171;
        float2x2 _246 = float2x2(float2(_12_colorGreen.y - 1.0f, _159 - 1.0f), float2(_165 - 1.0f, _171 - 1.0f));
        _248 = test_bifffff22(_234, _235, _236, _237, _238, _246);
    }
    else
    {
        _248 = false;
    }
    bool _266 = false;
    if (_248)
    {
        int _252 = 3;
        float _253 = _12_colorGreen.y;
        float _254 = _159;
        float _255 = _165;
        float _256 = _171;
        float2x2 _264 = float2x2(float2(_12_colorGreen.y * 2.0f, _159 * 2.0f), float2(_165 * 2.0f, _171 * 2.0f));
        _266 = test_bifffff22(_252, _253, _254, _255, _256, _264);
    }
    else
    {
        _266 = false;
    }
    bool _284 = false;
    if (_266)
    {
        int _270 = 4;
        float _271 = _12_colorGreen.y;
        float _272 = _159;
        float _273 = _165;
        float _274 = _171;
        float2x2 _282 = float2x2(float2(_12_colorGreen.y * 0.5f, _159 * 0.5f), float2(_165 * 0.5f, _171 * 0.5f));
        _284 = test_bifffff22(_270, _271, _272, _273, _274, _282);
    }
    else
    {
        _284 = false;
    }
    bool _288 = false;
    if (_284)
    {
        _288 = divisionTest_b();
    }
    else
    {
        _288 = false;
    }
    float4 _289 = 0.0f.xxxx;
    if (_288)
    {
        _289 = _12_colorGreen;
    }
    else
    {
        _289 = _12_colorRed;
    }
    return _289;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
