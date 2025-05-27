cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_src : packoffset(c0);
    float4 _7_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float _28 = 0.0f;
    if ((2.0f * _7_src.x) <= _7_src.w)
    {
        _28 = (2.0f * _7_dst.x) * _7_src.x;
    }
    else
    {
        _28 = (_7_dst.w * _7_src.w) - ((2.0f * (_7_src.w - _7_src.x)) * (_7_dst.w - _7_dst.x));
    }
    float _75 = 0.0f;
    if ((2.0f * _7_src.y) <= _7_src.w)
    {
        _75 = (2.0f * _7_dst.y) * _7_src.y;
    }
    else
    {
        _75 = (_7_dst.w * _7_src.w) - ((2.0f * (_7_src.w - _7_src.y)) * (_7_dst.w - _7_dst.y));
    }
    float _120 = 0.0f;
    if ((2.0f * _7_src.z) <= _7_src.w)
    {
        _120 = (2.0f * _7_dst.z) * _7_src.z;
    }
    else
    {
        _120 = (_7_dst.w * _7_src.w) - ((2.0f * (_7_src.w - _7_src.z)) * (_7_dst.w - _7_dst.z));
    }
    float4 _RESERVED_IDENTIFIER_FIXUP_0_result = float4(_28, _75, _120, _7_dst.w + ((1.0f - _7_dst.w) * _7_src.w));
    float4 _171 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float3 _191 = _171.xyz + ((_7_src.xyz * (1.0f - _7_dst.w)) + (_7_dst.xyz * (1.0f - _7_src.w)));
    float4 _192 = _RESERVED_IDENTIFIER_FIXUP_0_result;
    float4 _193 = float4(_191.x, _191.y, _191.z, _192.w);
    _RESERVED_IDENTIFIER_FIXUP_0_result = _193;
    sk_FragColor = _193;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
