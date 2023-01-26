cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    int _10_mask : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool ok = true;
    bool _41 = false;
    if (true)
    {
        _41 = !(_10_colorGreen.y == 1.0f);
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    uint2 _55 = uint2(uint(_10_mask), ~uint(_10_mask));
    uint2 mask = _55;
    uint2 _59 = ~_55;
    int2 _64 = int2(int(_59.x), int(_59.y));
    int2 imask = _64;
    int2 _66 = ~_64;
    uint2 _72 = (~_55) & uint2(uint(_66.x), uint(_66.y));
    mask = _72;
    float _77 = float(_72.x);
    float4 _78 = float4(_77, 0.0f, 0.0f, 0.0f);
    float4 _79 = float4(0.0f, _77, 0.0f, 0.0f);
    float4 _80 = float4(0.0f, 0.0f, _77, 0.0f);
    float4 _81 = float4(0.0f, 0.0f, 0.0f, _77);
    float4x4 m = float4x4(_78, _79, _80, _81);
    float4 _88 = 0.0f.xxxx;
    if (!(_10_colorGreen.x == 1.0f))
    {
        _88 = mul(-_10_colorGreen, float4x4(-_78, -_79, -_80, -_81));
    }
    else
    {
        _88 = _10_colorRed;
    }
    return _88;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
