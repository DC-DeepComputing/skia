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
    float _41 = float((_15_dst.x == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_0_dxScale = _41;
    float _57 = 0.0f;
    if (abs(_15_src.w - _15_src.x) >= 6.103515625e-05f)
    {
        float _68 = _15_dst.x * _15_src.w;
        float _76 = _15_src.w - _15_src.x;
        _57 = guarded_divide_Qhhh(_68, _76);
    }
    else
    {
        _57 = _15_dst.w;
    }
    float _82 = _41 * min(_15_dst.w, _57);
    float _RESERVED_IDENTIFIER_FIXUP_1_delta = _82;
    float _89 = float((_15_dst.y == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_2_dxScale = _89;
    float _104 = 0.0f;
    if (abs(_15_src.w - _15_src.y) >= 6.103515625e-05f)
    {
        float _115 = _15_dst.y * _15_src.w;
        float _123 = _15_src.w - _15_src.y;
        _104 = guarded_divide_Qhhh(_115, _123);
    }
    else
    {
        _104 = _15_dst.w;
    }
    float _129 = _89 * min(_15_dst.w, _104);
    float _RESERVED_IDENTIFIER_FIXUP_3_delta = _129;
    float _136 = float((_15_dst.z == 0.0f) ? 0 : 1);
    float _RESERVED_IDENTIFIER_FIXUP_4_dxScale = _136;
    float _151 = 0.0f;
    if (abs(_15_src.w - _15_src.z) >= 6.103515625e-05f)
    {
        float _162 = _15_dst.z * _15_src.w;
        float _170 = _15_src.w - _15_src.z;
        _151 = guarded_divide_Qhhh(_162, _170);
    }
    else
    {
        _151 = _15_dst.w;
    }
    float _176 = _136 * min(_15_dst.w, _151);
    float _RESERVED_IDENTIFIER_FIXUP_5_delta = _176;
    sk_FragColor = float4(((_82 * _15_src.w) + (_15_src.x * (1.0f - _15_dst.w))) + (_15_dst.x * (1.0f - _15_src.w)), ((_129 * _15_src.w) + (_15_src.y * (1.0f - _15_dst.w))) + (_15_dst.y * (1.0f - _15_src.w)), ((_176 * _15_src.w) + (_15_src.z * (1.0f - _15_dst.w))) + (_15_dst.z * (1.0f - _15_src.w)), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
