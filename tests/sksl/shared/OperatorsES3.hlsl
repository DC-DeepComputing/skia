cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float _7_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float x = 1.0f;
    float y = 2.0f;
    int z = 3;
    float _37 = (1.0f - 1.0f) + (((2.0f * 1.0f) * 1.0f) * (2.0f - 1.0f));
    x = _37;
    float _39 = (_37 / 2.0f) / _37;
    y = _39;
    int _47 = ((((3 / 2) % 3) << 4) >> 2) << 1;
    z = _47;
    bool _67 = false;
    if ((_37 > 4.0f) == (_37 < 2.0f))
    {
        _67 = true;
    }
    else
    {
        bool _66 = false;
        if (2.0f >= _7_unknownInput)
        {
            _66 = _39 <= _37;
        }
        else
        {
            _66 = false;
        }
        _67 = _66;
    }
    bool b = _67;
    bool _71 = _7_unknownInput > 2.0f;
    bool c = _71;
    bool _73 = _67 != _71;
    bool d = _73;
    bool _77 = false;
    if (_67)
    {
        _77 = _71;
    }
    else
    {
        _77 = false;
    }
    bool e = _77;
    bool _81 = false;
    if (_67)
    {
        _81 = true;
    }
    else
    {
        _81 = _71;
    }
    bool f = _81;
    float _83 = _37 + 12.0f;
    x = _83;
    float _84 = _83 - 12.0f;
    x = _84;
    float _86 = _39 * 0.100000001490116119384765625f;
    y = _86;
    x = _84 * _86;
    x = 6.0f;
    y = (((float(_67) * float(_71)) * float(_73)) * float(_77)) * float(_81);
    int _99 = _47 | 0;
    z = _99;
    int _101 = _99 & (-1);
    z = _101;
    int _102 = _101 ^ 0;
    z = _102;
    int _103 = _102 >> 2;
    z = _103;
    int _104 = _103 << 4;
    z = _104;
    z = _104 % 5;
    y = 6.0f;
    z = 6;
    int2 w = int2(-6, -6);
    int2 _113 = ~int2(-6, -6);
    w = _113;
    bool _120 = false;
    if (_113.x == 5)
    {
        _120 = _113.y == 5;
    }
    else
    {
        _120 = false;
    }
    bool _123 = false;
    if (_120)
    {
        _123 = true;
    }
    else
    {
        _123 = false;
    }
    bool _126 = false;
    if (_123)
    {
        _126 = true;
    }
    else
    {
        _126 = false;
    }
    bool _129 = false;
    if (_126)
    {
        _129 = true;
    }
    else
    {
        _129 = false;
    }
    float4 _130 = 0.0f.xxxx;
    if (_129)
    {
        _130 = _7_colorGreen;
    }
    else
    {
        _130 = _7_colorRed;
    }
    return _130;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
