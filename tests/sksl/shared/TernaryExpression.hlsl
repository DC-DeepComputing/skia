cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool ok = true;
    bool _40 = false;
    if (true)
    {
        _40 = _10_colorGreen.y == 1.0f;
    }
    else
    {
        _40 = false;
    }
    ok = _40;
    bool _48 = false;
    if (_40)
    {
        _48 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _48 = false;
    }
    ok = _48;
    bool _61 = false;
    if (_48)
    {
        _61 = all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y));
    }
    else
    {
        _61 = false;
    }
    ok = _61;
    bool _73 = false;
    if (_61)
    {
        _73 = !any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y));
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    float4 _74 = 0.0f.xxxx;
    if (_73)
    {
        _74 = _10_colorGreen;
    }
    else
    {
        _74 = _10_colorRed;
    }
    return _74;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
