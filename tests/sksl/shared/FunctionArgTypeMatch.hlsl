cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool _28 = false;
    if (true)
    {
        _28 = true;
    }
    else
    {
        _28 = false;
    }
    bool _31 = false;
    if (_28)
    {
        _31 = true;
    }
    else
    {
        _31 = false;
    }
    bool _34 = false;
    if (_31)
    {
        _34 = true;
    }
    else
    {
        _34 = false;
    }
    bool _37 = false;
    if (_34)
    {
        _37 = true;
    }
    else
    {
        _37 = false;
    }
    bool _40 = false;
    if (_37)
    {
        _40 = true;
    }
    else
    {
        _40 = false;
    }
    bool _43 = false;
    if (_40)
    {
        _43 = true;
    }
    else
    {
        _43 = false;
    }
    bool _46 = false;
    if (_43)
    {
        _46 = true;
    }
    else
    {
        _46 = false;
    }
    bool _49 = false;
    if (_46)
    {
        _49 = true;
    }
    else
    {
        _49 = false;
    }
    bool _52 = false;
    if (_49)
    {
        _52 = true;
    }
    else
    {
        _52 = false;
    }
    bool _55 = false;
    if (_52)
    {
        _55 = true;
    }
    else
    {
        _55 = false;
    }
    bool _58 = false;
    if (_55)
    {
        _58 = true;
    }
    else
    {
        _58 = false;
    }
    bool _61 = false;
    if (_58)
    {
        _61 = true;
    }
    else
    {
        _61 = false;
    }
    bool _64 = false;
    if (_61)
    {
        _64 = true;
    }
    else
    {
        _64 = false;
    }
    bool _67 = false;
    if (_64)
    {
        _67 = true;
    }
    else
    {
        _67 = false;
    }
    bool _70 = false;
    if (_67)
    {
        _70 = true;
    }
    else
    {
        _70 = false;
    }
    bool _73 = false;
    if (_70)
    {
        _73 = true;
    }
    else
    {
        _73 = false;
    }
    bool _76 = false;
    if (_73)
    {
        _76 = true;
    }
    else
    {
        _76 = false;
    }
    bool _79 = false;
    if (_76)
    {
        _79 = true;
    }
    else
    {
        _79 = false;
    }
    bool _82 = false;
    if (_79)
    {
        _82 = true;
    }
    else
    {
        _82 = false;
    }
    bool _85 = false;
    if (_82)
    {
        _85 = true;
    }
    else
    {
        _85 = false;
    }
    bool _88 = false;
    if (_85)
    {
        _88 = true;
    }
    else
    {
        _88 = false;
    }
    bool _91 = false;
    if (_88)
    {
        _91 = true;
    }
    else
    {
        _91 = false;
    }
    bool _94 = false;
    if (_91)
    {
        _94 = true;
    }
    else
    {
        _94 = false;
    }
    float4 _95 = 0.0f.xxxx;
    if (_94)
    {
        _95 = _7_colorGreen;
    }
    else
    {
        _95 = _7_colorRed;
    }
    return _95;
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
