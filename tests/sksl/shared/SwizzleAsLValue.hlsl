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
    float4 color = _10_colorGreen * 0.5f;
    color.w = 2.0f;
    color.y *= 4.0f;
    float3 _49 = color.yzw * 0.5f.xxx;
    color = float4(color.x, _49.x, _49.y, _49.z);
    float4 _52 = color;
    float4 _57 = _52.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _58 = color;
    float4 _59 = float4(_57.w, _57.y, _57.x, _57.z);
    color = _59;
    float _65 = 0.0f;
    if (_59.w <= 1.0f)
    {
        _65 = _59.z;
    }
    else
    {
        _65 = 0.0f;
    }
    color.x += _65;
    float4 _77 = 0.0f.xxxx;
    if (all(bool4(color.x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, color.y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, color.z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, color.w == float4(1.0f, 1.0f, 0.25f, 1.0f).w)))
    {
        _77 = _10_colorGreen;
    }
    else
    {
        _77 = _10_colorRed;
    }
    return _77;
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
