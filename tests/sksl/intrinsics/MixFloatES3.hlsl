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
    bool _45 = _42.x;
    bool4 _52 = bool4(!_45, !_42.y, !_42.z, !_42.w);
    bool4 TFTF = _52;
    bool _98 = false;
    if ((_45 ? _10_colorWhite.x : _10_colorBlack.x) == _10_colorBlack.x)
    {
        bool2 _90 = _42.xy;
        float2 _75 = float2(_90.x ? _10_colorWhite.xy.x : _10_colorBlack.xy.x, _90.y ? _10_colorWhite.xy.y : _10_colorBlack.xy.y);
        float2 _95 = float2(_10_colorBlack.x, 1.0f);
        _98 = all(bool2(_75.x == _95.x, _75.y == _95.y));
    }
    else
    {
        _98 = false;
    }
    bool _127 = false;
    if (_98)
    {
        bool3 _117 = _42.xyz;
        float3 _101 = float3(_117.x ? _10_colorWhite.xyz.x : _10_colorBlack.xyz.x, _117.y ? _10_colorWhite.xyz.y : _10_colorBlack.xyz.y, _117.z ? _10_colorWhite.xyz.z : _10_colorBlack.xyz.z);
        float3 _124 = float3(_10_colorBlack.x, 1.0f, _10_colorBlack.z);
        _127 = all(bool3(_101.x == _124.x, _101.y == _124.y, _101.z == _124.z));
    }
    else
    {
        _127 = false;
    }
    bool _148 = false;
    if (_127)
    {
        float4 _130 = float4(_42.x ? _10_colorWhite.x : _10_colorBlack.x, _42.y ? _10_colorWhite.y : _10_colorBlack.y, _42.z ? _10_colorWhite.z : _10_colorBlack.z, _42.w ? _10_colorWhite.w : _10_colorBlack.w);
        float4 _145 = float4(_10_colorBlack.x, 1.0f, _10_colorBlack.z, 1.0f);
        _148 = all(bool4(_130.x == _145.x, _130.y == _145.y, _130.z == _145.z, _130.w == _145.w));
    }
    else
    {
        _148 = false;
    }
    bool _170 = false;
    if (_148)
    {
        _170 = (_52.x ? _10_testInputs.x : _10_colorWhite.x) == _10_testInputs.x;
    }
    else
    {
        _170 = false;
    }
    bool _194 = false;
    if (_170)
    {
        bool2 _187 = _52.xy;
        float2 _173 = float2(_187.x ? _10_testInputs.xy.x : _10_colorWhite.xy.x, _187.y ? _10_testInputs.xy.y : _10_colorWhite.xy.y);
        float2 _191 = float2(_10_testInputs.x, 1.0f);
        _194 = all(bool2(_173.x == _191.x, _173.y == _191.y));
    }
    else
    {
        _194 = false;
    }
    bool _221 = false;
    if (_194)
    {
        bool3 _211 = _52.xyz;
        float3 _197 = float3(_211.x ? _10_testInputs.xyz.x : _10_colorWhite.xyz.x, _211.y ? _10_testInputs.xyz.y : _10_colorWhite.xyz.y, _211.z ? _10_testInputs.xyz.z : _10_colorWhite.xyz.z);
        float3 _218 = float3(_10_testInputs.x, 1.0f, _10_testInputs.z);
        _221 = all(bool3(_197.x == _218.x, _197.y == _218.y, _197.z == _218.z));
    }
    else
    {
        _221 = false;
    }
    bool _242 = false;
    if (_221)
    {
        float4 _224 = float4(_52.x ? _10_testInputs.x : _10_colorWhite.x, _52.y ? _10_testInputs.y : _10_colorWhite.y, _52.z ? _10_testInputs.z : _10_colorWhite.z, _52.w ? _10_testInputs.w : _10_colorWhite.w);
        float4 _239 = float4(_10_testInputs.x, 1.0f, _10_testInputs.z, 1.0f);
        _242 = all(bool4(_224.x == _239.x, _224.y == _239.y, _224.z == _239.z, _224.w == _239.w));
    }
    else
    {
        _242 = false;
    }
    float4 _243 = 0.0f.xxxx;
    if (_242)
    {
        _243 = _10_colorGreen;
    }
    else
    {
        _243 = _10_colorRed;
    }
    return _243;
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
