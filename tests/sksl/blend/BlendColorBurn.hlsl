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

float color_burn_component_Qhh2h2(float2 _25, float2 _26)
{
    float _35 = 0.0f;
    if (_26.y == _26.x)
    {
        _35 = _26.y;
    }
    else
    {
        _35 = 0.0f;
    }
    float dyTerm = _35;
    float _48 = 0.0f;
    if (abs(_25.x) >= 6.103515625e-05f)
    {
        float _65 = (_26.y - _26.x) * _25.y;
        float _68 = _25.x;
        _48 = _26.y - min(_26.y, guarded_divide_Qhhh(_65, _68));
    }
    else
    {
        _48 = _35;
    }
    float delta = _48;
    return ((_48 * _25.y) + (_25.x * (1.0f - _26.y))) + (_26.x * (1.0f - _25.y));
}

void frag_main()
{
    float2 _99 = _9_src.xw;
    float2 _104 = _9_dst.xw;
    float2 _109 = _9_src.yw;
    float2 _113 = _9_dst.yw;
    float2 _118 = _9_src.zw;
    float2 _122 = _9_dst.zw;
    sk_FragColor = float4(color_burn_component_Qhh2h2(_99, _104), color_burn_component_Qhh2h2(_109, _113), color_burn_component_Qhh2h2(_118, _122), _9_src.w + ((1.0f - _9_src.w) * _9_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
