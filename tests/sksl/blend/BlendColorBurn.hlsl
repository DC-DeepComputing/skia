cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_src : packoffset(c0);
    float4 _15_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float guarded_divide_Qhhh(float _20, float _21)
{
    return _20 / (_21 + _kGuardedDivideEpsilon);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float _42 = 0.0f;
    if (_15_dst.w == _15_dst.x)
    {
        _42 = _15_dst.w;
    }
    else
    {
        _42 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_0_dyTerm = _42;
    float _58 = 0.0f;
    if (abs(_15_src.x) >= 6.103515625e-05f)
    {
        float _80 = (_15_dst.w - _15_dst.x) * _15_src.w;
        float _84 = _15_src.x;
        _58 = _15_dst.w - min(_15_dst.w, guarded_divide_Qhhh(_80, _84));
    }
    else
    {
        _58 = _42;
    }
    float _RESERVED_IDENTIFIER_FIXUP_1_delta = _58;
    float _96 = 0.0f;
    if (_15_dst.w == _15_dst.y)
    {
        _96 = _15_dst.w;
    }
    else
    {
        _96 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_2_dyTerm = _96;
    float _110 = 0.0f;
    if (abs(_15_src.y) >= 6.103515625e-05f)
    {
        float _132 = (_15_dst.w - _15_dst.y) * _15_src.w;
        float _136 = _15_src.y;
        _110 = _15_dst.w - min(_15_dst.w, guarded_divide_Qhhh(_132, _136));
    }
    else
    {
        _110 = _96;
    }
    float _RESERVED_IDENTIFIER_FIXUP_3_delta = _110;
    float _148 = 0.0f;
    if (_15_dst.w == _15_dst.z)
    {
        _148 = _15_dst.w;
    }
    else
    {
        _148 = 0.0f;
    }
    float _RESERVED_IDENTIFIER_FIXUP_4_dyTerm = _148;
    float _162 = 0.0f;
    if (abs(_15_src.z) >= 6.103515625e-05f)
    {
        float _184 = (_15_dst.w - _15_dst.z) * _15_src.w;
        float _188 = _15_src.z;
        _162 = _15_dst.w - min(_15_dst.w, guarded_divide_Qhhh(_184, _188));
    }
    else
    {
        _162 = _148;
    }
    float _RESERVED_IDENTIFIER_FIXUP_5_delta = _162;
    sk_FragColor = float4(((_58 * _15_src.w) + (_15_src.x * (1.0f - _15_dst.w))) + (_15_dst.x * (1.0f - _15_src.w)), ((_110 * _15_src.w) + (_15_src.y * (1.0f - _15_dst.w))) + (_15_dst.y * (1.0f - _15_src.w)), ((_162 * _15_src.w) + (_15_src.z * (1.0f - _15_dst.w))) + (_15_dst.z * (1.0f - _15_src.w)), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
