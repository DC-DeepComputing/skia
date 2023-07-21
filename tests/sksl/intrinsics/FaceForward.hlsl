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

float spvFaceForward(float n, float i, float nref)
{
    return i * nref < 0.0 ? n : -n;
}

float4 main(float2 _24)
{
    float _28 = spvFaceForward(1.0f, 1000000015047466219876688855040.0f, 1000000015047466219876688855040.0f);
    float huge = _28;
    float2 _32 = faceforward(1.0f.xx, 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    float2 huge2 = _32;
    float3 _38 = faceforward(1.0f.xxx, 1000000015047466219876688855040.0f.xxx, 1000000015047466219876688855040.0f.xxx);
    float3 huge3 = _38;
    float4 _43 = faceforward(1.0f.xxxx, 1000000015047466219876688855040.0f.xxxx, 1000000015047466219876688855040.0f.xxxx);
    float4 huge4 = _43;
    float4 expectedPos = _28.xxxx + _32.xxxx;
    float4 expectedNeg = _38.xxxx + _43.xxxx;
    float4 _65 = _10_colorGreen.yyyy * float4(1.0f, 2.0f, 3.0f, 4.0f);
    float4 N = _65;
    float4 _72 = _10_colorGreen.yyyy * float4(1.0f, 1.0f, -100.0f, 1.0f);
    float4 I = _72;
    float4 NRef = _10_colorGreen.yyyy;
    expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
    expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
    bool _98 = false;
    if (spvFaceForward(_65.x, _72.x, _10_colorGreen.yyyy.x) == (-1.0f))
    {
        float2 _90 = faceforward(_65.xy, _72.xy, _10_colorGreen.yyyy.xy);
        _98 = all(bool2(_90.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, _90.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _98 = false;
    }
    bool _109 = false;
    if (_98)
    {
        float3 _101 = faceforward(_65.xyz, _72.xyz, _10_colorGreen.yyyy.xyz);
        _109 = all(bool3(_101.x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, _101.y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, _101.z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _109 = false;
    }
    bool _116 = false;
    if (_109)
    {
        float4 _112 = faceforward(_65, _72, _10_colorGreen.yyyy);
        _116 = all(bool4(_112.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _112.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _112.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _112.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _116 = false;
    }
    bool _120 = false;
    if (_116)
    {
        _120 = true;
    }
    else
    {
        _120 = false;
    }
    bool _127 = false;
    if (_120)
    {
        _127 = all(bool2(float2(-1.0f, -2.0f).x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, float2(-1.0f, -2.0f).y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _127 = false;
    }
    bool _134 = false;
    if (_127)
    {
        _134 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, float3(1.0f, 2.0f, 3.0f).y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, float3(1.0f, 2.0f, 3.0f).z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _134 = false;
    }
    bool _137 = false;
    if (_134)
    {
        _137 = true;
    }
    else
    {
        _137 = false;
    }
    float4 _138 = 0.0f.xxxx;
    if (_137)
    {
        _138 = _10_colorGreen;
    }
    else
    {
        _138 = _10_colorRed;
    }
    return _138;
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
