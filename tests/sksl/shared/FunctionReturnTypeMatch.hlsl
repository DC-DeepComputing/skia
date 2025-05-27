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
    float x1 = 1.0f;
    float2 x2 = 2.0f.xx;
    float3 x3 = 3.0f.xxx;
    float4 x4 = 4.0f.xxxx;
    float2x2 x5 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    float3x3 x6 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    float4x4 x7 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    float x8 = 1.0f;
    float2 x9 = 2.0f.xx;
    float3 x10 = 3.0f.xxx;
    float4 x11 = 4.0f.xxxx;
    float2x2 x12 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    float3x3 x13 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    float4x4 x14 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    bool x15 = true;
    bool2 x16 = bool2(true, true);
    bool3 x17 = bool3(true, true, true);
    bool4 x18 = bool4(true, true, true, true);
    int x19 = 1;
    int2 x20 = int2(2, 2);
    int3 x21 = int3(3, 3, 3);
    int4 x22 = int4(4, 4, 4, 4);
    bool _104 = false;
    if (true)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    bool _107 = false;
    if (_104)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    bool _110 = false;
    if (_107)
    {
        _110 = true;
    }
    else
    {
        _110 = false;
    }
    bool _118 = false;
    if (_110)
    {
        _118 = all(bool2(float2(2.0f, 0.0f).x == float2(2.0f, 0.0f).x, float2(2.0f, 0.0f).y == float2(2.0f, 0.0f).y)) && all(bool2(float2(0.0f, 2.0f).x == float2(0.0f, 2.0f).x, float2(0.0f, 2.0f).y == float2(0.0f, 2.0f).y));
    }
    else
    {
        _118 = false;
    }
    bool _129 = false;
    if (_118)
    {
        _129 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == float3(3.0f, 0.0f, 0.0f).x, float3(3.0f, 0.0f, 0.0f).y == float3(3.0f, 0.0f, 0.0f).y, float3(3.0f, 0.0f, 0.0f).z == float3(3.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == float3(0.0f, 3.0f, 0.0f).x, float3(0.0f, 3.0f, 0.0f).y == float3(0.0f, 3.0f, 0.0f).y, float3(0.0f, 3.0f, 0.0f).z == float3(0.0f, 3.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == float3(0.0f, 0.0f, 3.0f).x, float3(0.0f, 0.0f, 3.0f).y == float3(0.0f, 0.0f, 3.0f).y, float3(0.0f, 0.0f, 3.0f).z == float3(0.0f, 0.0f, 3.0f).z));
    }
    else
    {
        _129 = false;
    }
    bool _143 = false;
    if (_129)
    {
        _143 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == float4(4.0f, 0.0f, 0.0f, 0.0f).x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == float4(4.0f, 0.0f, 0.0f, 0.0f).y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == float4(4.0f, 0.0f, 0.0f, 0.0f).z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == float4(4.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == float4(0.0f, 4.0f, 0.0f, 0.0f).x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == float4(0.0f, 4.0f, 0.0f, 0.0f).y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == float4(0.0f, 4.0f, 0.0f, 0.0f).z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == float4(0.0f, 4.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == float4(0.0f, 0.0f, 4.0f, 0.0f).x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == float4(0.0f, 0.0f, 4.0f, 0.0f).y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == float4(0.0f, 0.0f, 4.0f, 0.0f).z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == float4(0.0f, 0.0f, 4.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == float4(0.0f, 0.0f, 0.0f, 4.0f).x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == float4(0.0f, 0.0f, 0.0f, 4.0f).y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == float4(0.0f, 0.0f, 0.0f, 4.0f).z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == float4(0.0f, 0.0f, 0.0f, 4.0f).w));
    }
    else
    {
        _143 = false;
    }
    bool _146 = false;
    if (_143)
    {
        _146 = true;
    }
    else
    {
        _146 = false;
    }
    bool _149 = false;
    if (_146)
    {
        _149 = true;
    }
    else
    {
        _149 = false;
    }
    bool _152 = false;
    if (_149)
    {
        _152 = true;
    }
    else
    {
        _152 = false;
    }
    bool _155 = false;
    if (_152)
    {
        _155 = true;
    }
    else
    {
        _155 = false;
    }
    bool _163 = false;
    if (_155)
    {
        _163 = all(bool2(float2(2.0f, 0.0f).x == float2(2.0f, 0.0f).x, float2(2.0f, 0.0f).y == float2(2.0f, 0.0f).y)) && all(bool2(float2(0.0f, 2.0f).x == float2(0.0f, 2.0f).x, float2(0.0f, 2.0f).y == float2(0.0f, 2.0f).y));
    }
    else
    {
        _163 = false;
    }
    bool _174 = false;
    if (_163)
    {
        _174 = (all(bool3(float3(3.0f, 0.0f, 0.0f).x == float3(3.0f, 0.0f, 0.0f).x, float3(3.0f, 0.0f, 0.0f).y == float3(3.0f, 0.0f, 0.0f).y, float3(3.0f, 0.0f, 0.0f).z == float3(3.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 3.0f, 0.0f).x == float3(0.0f, 3.0f, 0.0f).x, float3(0.0f, 3.0f, 0.0f).y == float3(0.0f, 3.0f, 0.0f).y, float3(0.0f, 3.0f, 0.0f).z == float3(0.0f, 3.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 3.0f).x == float3(0.0f, 0.0f, 3.0f).x, float3(0.0f, 0.0f, 3.0f).y == float3(0.0f, 0.0f, 3.0f).y, float3(0.0f, 0.0f, 3.0f).z == float3(0.0f, 0.0f, 3.0f).z));
    }
    else
    {
        _174 = false;
    }
    bool _188 = false;
    if (_174)
    {
        _188 = ((all(bool4(float4(4.0f, 0.0f, 0.0f, 0.0f).x == float4(4.0f, 0.0f, 0.0f, 0.0f).x, float4(4.0f, 0.0f, 0.0f, 0.0f).y == float4(4.0f, 0.0f, 0.0f, 0.0f).y, float4(4.0f, 0.0f, 0.0f, 0.0f).z == float4(4.0f, 0.0f, 0.0f, 0.0f).z, float4(4.0f, 0.0f, 0.0f, 0.0f).w == float4(4.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 4.0f, 0.0f, 0.0f).x == float4(0.0f, 4.0f, 0.0f, 0.0f).x, float4(0.0f, 4.0f, 0.0f, 0.0f).y == float4(0.0f, 4.0f, 0.0f, 0.0f).y, float4(0.0f, 4.0f, 0.0f, 0.0f).z == float4(0.0f, 4.0f, 0.0f, 0.0f).z, float4(0.0f, 4.0f, 0.0f, 0.0f).w == float4(0.0f, 4.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 4.0f, 0.0f).x == float4(0.0f, 0.0f, 4.0f, 0.0f).x, float4(0.0f, 0.0f, 4.0f, 0.0f).y == float4(0.0f, 0.0f, 4.0f, 0.0f).y, float4(0.0f, 0.0f, 4.0f, 0.0f).z == float4(0.0f, 0.0f, 4.0f, 0.0f).z, float4(0.0f, 0.0f, 4.0f, 0.0f).w == float4(0.0f, 0.0f, 4.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 0.0f, 4.0f).x == float4(0.0f, 0.0f, 0.0f, 4.0f).x, float4(0.0f, 0.0f, 0.0f, 4.0f).y == float4(0.0f, 0.0f, 0.0f, 4.0f).y, float4(0.0f, 0.0f, 0.0f, 4.0f).z == float4(0.0f, 0.0f, 0.0f, 4.0f).z, float4(0.0f, 0.0f, 0.0f, 4.0f).w == float4(0.0f, 0.0f, 0.0f, 4.0f).w));
    }
    else
    {
        _188 = false;
    }
    bool _191 = false;
    if (_188)
    {
        _191 = true;
    }
    else
    {
        _191 = false;
    }
    bool _194 = false;
    if (_191)
    {
        _194 = true;
    }
    else
    {
        _194 = false;
    }
    bool _197 = false;
    if (_194)
    {
        _197 = true;
    }
    else
    {
        _197 = false;
    }
    bool _200 = false;
    if (_197)
    {
        _200 = true;
    }
    else
    {
        _200 = false;
    }
    bool _203 = false;
    if (_200)
    {
        _203 = true;
    }
    else
    {
        _203 = false;
    }
    bool _206 = false;
    if (_203)
    {
        _206 = true;
    }
    else
    {
        _206 = false;
    }
    bool _209 = false;
    if (_206)
    {
        _209 = true;
    }
    else
    {
        _209 = false;
    }
    bool _212 = false;
    if (_209)
    {
        _212 = true;
    }
    else
    {
        _212 = false;
    }
    float4 _213 = 0.0f.xxxx;
    if (_212)
    {
        _213 = _7_colorGreen;
    }
    else
    {
        _213 = _7_colorRed;
    }
    return _213;
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
