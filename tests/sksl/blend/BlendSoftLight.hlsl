cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_src : packoffset(c0);
    float4 _8_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float soft_light_component_Qhh2h2(float2 _14, float2 _15)
{
    if ((2.0f * _14.x) <= _14.y)
    {
        return ((((_15.x * _15.x) * (_14.y - (2.0f * _14.x))) / (_15.y + 6.103515625e-05f)) + ((1.0f - _15.y) * _14.x)) + (_15.x * (((-_14.y) + (2.0f * _14.x)) + 1.0f));
    }
    else
    {
        if ((4.0f * _15.x) <= _15.y)
        {
            float _81 = _15.x * _15.x;
            float DSqd = _81;
            float _85 = _81 * _15.x;
            float DCub = _85;
            float _91 = _15.y * _15.y;
            float DaSqd = _91;
            float _95 = _91 * _15.y;
            float DaCub = _95;
            return ((((_91 * (_14.x - (_15.x * (((3.0f * _14.y) - (6.0f * _14.x)) - 1.0f)))) + (((12.0f * _15.y) * _81) * (_14.y - (2.0f * _14.x)))) - ((16.0f * _85) * (_14.y - (2.0f * _14.x)))) - (_95 * _14.x)) / (_91 + 6.103515625e-05f);
        }
        else
        {
            return (((_15.x * ((_14.y - (2.0f * _14.x)) + 1.0f)) + _14.x) - (sqrt(_15.y * _15.x) * (_14.y - (2.0f * _14.x)))) - (_15.y * _14.x);
        }
    }
}

void frag_main()
{
    float4 _186 = 0.0f.xxxx;
    if (_8_dst.w == 0.0f)
    {
        _186 = _8_src;
    }
    else
    {
        float2 _197 = _8_src.xw;
        float2 _201 = _8_dst.xw;
        float2 _206 = _8_src.yw;
        float2 _210 = _8_dst.yw;
        float2 _215 = _8_src.zw;
        float2 _219 = _8_dst.zw;
        _186 = float4(soft_light_component_Qhh2h2(_197, _201), soft_light_component_Qhh2h2(_206, _210), soft_light_component_Qhh2h2(_215, _219), _8_src.w + ((1.0f - _8_src.w) * _8_dst.w));
    }
    sk_FragColor = _186;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
