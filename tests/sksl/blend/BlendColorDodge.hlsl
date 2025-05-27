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
    float _31 = float((_14_dst.x == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_0_dxScale = _31;
    float _47 = 0.0f;
    if (abs(_14_src.w - _14_src.x) >= 6.103515625e-05f)
    {
        _47 = (_14_dst.x * _14_src.w) / ((_14_src.w - _14_src.x) + _kGuardedDivideEpsilon);
    }
    else
    {
        _47 = _14_dst.w;
    }
    float _72 = _31 * min(_14_dst.w, _47);
    float _RESERVED_IDENTIFIER_FIXUP_1_delta = _72;
    float _79 = float((_14_dst.y == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_2_dxScale = _79;
    float _94 = 0.0f;
    if (abs(_14_src.w - _14_src.y) >= 6.103515625e-05f)
    {
        _94 = (_14_dst.y * _14_src.w) / ((_14_src.w - _14_src.y) + _kGuardedDivideEpsilon);
    }
    else
    {
        _94 = _14_dst.w;
    }
    float _119 = _79 * min(_14_dst.w, _94);
    float _RESERVED_IDENTIFIER_FIXUP_3_delta = _119;
    float _126 = float((_14_dst.z == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_4_dxScale = _126;
    float _141 = 0.0f;
    if (abs(_14_src.w - _14_src.z) >= 6.103515625e-05f)
    {
        _141 = (_14_dst.z * _14_src.w) / ((_14_src.w - _14_src.z) + _kGuardedDivideEpsilon);
    }
    else
    {
        _141 = _14_dst.w;
    }
    float _166 = _126 * min(_14_dst.w, _141);
    float _RESERVED_IDENTIFIER_FIXUP_5_delta = _166;
    sk_FragColor = float4(((_72 * _14_src.w) + (_14_src.x * (1.0f - _14_dst.w))) + (_14_dst.x * (1.0f - _14_src.w)), ((_119 * _14_src.w) + (_14_src.y * (1.0f - _14_dst.w))) + (_14_dst.y * (1.0f - _14_src.w)), ((_166 * _14_src.w) + (_14_src.z * (1.0f - _14_dst.w))) + (_14_dst.z * (1.0f - _14_src.w)), _14_src.w + ((1.0f - _14_src.w) * _14_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
