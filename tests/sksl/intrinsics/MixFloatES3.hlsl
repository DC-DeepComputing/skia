cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float4 _10_colorBlack : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
    float4 _10_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 _42 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
    bool4 FTFT = _42;
    bool4 _55 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
    bool4 _44 = bool4(!_55.x, !_55.y, !_55.z, !_55.w);
    bool4 TFTF = _44;
    bool _102 = false;
    if ((_42.x ? _10_colorWhite.x : _10_colorBlack.x) == _10_colorBlack.x)
    {
        bool2 _94 = _42.xy;
        float2 _79 = float2(_94.x ? _10_colorWhite.xy.x : _10_colorBlack.xy.x, _94.y ? _10_colorWhite.xy.y : _10_colorBlack.xy.y);
        float2 _99 = float2(_10_colorBlack.x, 1.0f);
        _102 = all(bool2(_79.x == _99.x, _79.y == _99.y));
    }
    else
    {
        _102 = false;
    }
    bool _131 = false;
    if (_102)
    {
        bool3 _121 = _42.xyz;
        float3 _105 = float3(_121.x ? _10_colorWhite.xyz.x : _10_colorBlack.xyz.x, _121.y ? _10_colorWhite.xyz.y : _10_colorBlack.xyz.y, _121.z ? _10_colorWhite.xyz.z : _10_colorBlack.xyz.z);
        float3 _128 = float3(_10_colorBlack.x, 1.0f, _10_colorBlack.z);
        _131 = all(bool3(_105.x == _128.x, _105.y == _128.y, _105.z == _128.z));
    }
    else
    {
        _131 = false;
    }
    bool _152 = false;
    if (_131)
    {
        float4 _134 = float4(_42.x ? _10_colorWhite.x : _10_colorBlack.x, _42.y ? _10_colorWhite.y : _10_colorBlack.y, _42.z ? _10_colorWhite.z : _10_colorBlack.z, _42.w ? _10_colorWhite.w : _10_colorBlack.w);
        float4 _149 = float4(_10_colorBlack.x, 1.0f, _10_colorBlack.z, 1.0f);
        _152 = all(bool4(_134.x == _149.x, _134.y == _149.y, _134.z == _149.z, _134.w == _149.w));
    }
    else
    {
        _152 = false;
    }
    bool _174 = false;
    if (_152)
    {
        _174 = (_44.x ? _10_testInputs.x : _10_colorWhite.x) == _10_testInputs.x;
    }
    else
    {
        _174 = false;
    }
    bool _198 = false;
    if (_174)
    {
        bool2 _191 = _44.xy;
        float2 _177 = float2(_191.x ? _10_testInputs.xy.x : _10_colorWhite.xy.x, _191.y ? _10_testInputs.xy.y : _10_colorWhite.xy.y);
        float2 _195 = float2(_10_testInputs.x, 1.0f);
        _198 = all(bool2(_177.x == _195.x, _177.y == _195.y));
    }
    else
    {
        _198 = false;
    }
    bool _225 = false;
    if (_198)
    {
        bool3 _215 = _44.xyz;
        float3 _201 = float3(_215.x ? _10_testInputs.xyz.x : _10_colorWhite.xyz.x, _215.y ? _10_testInputs.xyz.y : _10_colorWhite.xyz.y, _215.z ? _10_testInputs.xyz.z : _10_colorWhite.xyz.z);
        float3 _222 = float3(_10_testInputs.x, 1.0f, _10_testInputs.z);
        _225 = all(bool3(_201.x == _222.x, _201.y == _222.y, _201.z == _222.z));
    }
    else
    {
        _225 = false;
    }
    bool _246 = false;
    if (_225)
    {
        float4 _228 = float4(_44.x ? _10_testInputs.x : _10_colorWhite.x, _44.y ? _10_testInputs.y : _10_colorWhite.y, _44.z ? _10_testInputs.z : _10_colorWhite.z, _44.w ? _10_testInputs.w : _10_colorWhite.w);
        float4 _243 = float4(_10_testInputs.x, 1.0f, _10_testInputs.z, 1.0f);
        _246 = all(bool4(_228.x == _243.x, _228.y == _243.y, _228.z == _243.z, _228.w == _243.w));
    }
    else
    {
        _246 = false;
    }
    float4 _247 = 0.0f.xxxx;
    if (_246)
    {
        _247 = _10_colorGreen;
    }
    else
    {
        _247 = _10_colorRed;
    }
    return _247;
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
