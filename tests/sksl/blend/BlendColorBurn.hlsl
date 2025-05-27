cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _14_src : packoffset(c0);
    float4 _14_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float _32 = 0.0f;
    if (_14_dst.w == _14_dst.x)
    {
        _32 = _14_dst.w;
    }
    else
    {
        _32 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_0_dyTerm = _32;
    float _48 = 0.0f;
    if (abs(_14_src.x) >= 6.103515625e-05f)
    {
        _48 = _14_dst.w - min(_14_dst.w, ((_14_dst.w - _14_dst.x) * _14_src.w) / (_14_src.x + _kGuardedDivideEpsilon));
    }
    else
    {
        _48 = _32;
    }
    float _RESERVED_IDENTIFIER_FIXUP_1_delta = _48;
    float _86 = 0.0f;
    if (_14_dst.w == _14_dst.y)
    {
        _86 = _14_dst.w;
    }
    else
    {
        _86 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_2_dyTerm = _86;
    float _100 = 0.0f;
    if (abs(_14_src.y) >= 6.103515625e-05f)
    {
        _100 = _14_dst.w - min(_14_dst.w, ((_14_dst.w - _14_dst.y) * _14_src.w) / (_14_src.y + _kGuardedDivideEpsilon));
    }
    else
    {
        _100 = _86;
    }
    float _RESERVED_IDENTIFIER_FIXUP_3_delta = _100;
    float _138 = 0.0f;
    if (_14_dst.w == _14_dst.z)
    {
        _138 = _14_dst.w;
    }
    else
    {
        _138 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_4_dyTerm = _138;
    float _152 = 0.0f;
    if (abs(_14_src.z) >= 6.103515625e-05f)
    {
        _152 = _14_dst.w - min(_14_dst.w, ((_14_dst.w - _14_dst.z) * _14_src.w) / (_14_src.z + _kGuardedDivideEpsilon));
    }
    else
    {
        _152 = _138;
    }
    float _RESERVED_IDENTIFIER_FIXUP_5_delta = _152;
    sk_FragColor = float4(((_48 * _14_src.w) + (_14_src.x * (1.0f - _14_dst.w))) + (_14_dst.x * (1.0f - _14_src.w)), ((_100 * _14_src.w) + (_14_src.y * (1.0f - _14_dst.w))) + (_14_dst.y * (1.0f - _14_src.w)), ((_152 * _14_src.w) + (_14_src.z * (1.0f - _14_dst.w))) + (_14_dst.z * (1.0f - _14_src.w)), _14_src.w + ((1.0f - _14_src.w) * _14_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
