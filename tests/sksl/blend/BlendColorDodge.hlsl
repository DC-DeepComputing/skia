cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_src : packoffset(c0);
    float4 _9_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float guarded_divide_Qhhh(float _14, float _15)
{
    return _14 / (_15 + 6.103515625e-05f);
}

float color_dodge_component_Qhh2h2(float2 _25, float2 _26)
{
    float _38 = float((_26.x == 0.0f) ? 0 : 1);
    float dxScale = _38;
    float _50 = 0.0f;
    if (abs(_25.y - _25.x) >= 6.103515625e-05f)
    {
        float _59 = _26.x * _25.y;
        float _65 = _25.y - _25.x;
        _50 = guarded_divide_Qhhh(_59, _65);
    }
    else
    {
        _50 = _26.y;
    }
    float _70 = _38 * min(_26.y, _50);
    float delta = _70;
    return ((_70 * _25.y) + (_25.x * (1.0f - _26.y))) + (_26.x * (1.0f - _25.y));
}

void frag_main()
{
    float2 _96 = _9_src.xw;
    float2 _100 = _9_dst.xw;
    float2 _105 = _9_src.yw;
    float2 _109 = _9_dst.yw;
    float2 _114 = _9_src.zw;
    float2 _118 = _9_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_96, _100), color_dodge_component_Qhh2h2(_105, _109), color_dodge_component_Qhh2h2(_114, _118), _9_src.w + ((1.0f - _9_src.w) * _9_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
