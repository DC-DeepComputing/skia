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

float blend_color_saturation_Qhh3(float3 _21)
{
    return max(max(_21.x, _21.y), _21.z) - min(min(_21.x, _21.y), _21.z);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float _55 = _15_dst.w * _15_src.w;
    float _RESERVED_IDENTIFIER_FIXUP_0_alpha = _55;
    float3 _63 = _15_src.xyz * _15_dst.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_sda = _63;
    float3 _71 = _15_dst.xyz * _15_src.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_2_dsa = _71;
    float3 _RESERVED_IDENTIFIER_FIXUP_3_l = _71;
    float3 _RESERVED_IDENTIFIER_FIXUP_4_r = _63;
    float _77 = _71.x;
    float _78 = _71.y;
    float _79 = _71.z;
    float _75 = min(min(_77, _78), _79);
    float _RESERVED_IDENTIFIER_FIXUP_5_mn = _75;
    float _81 = max(max(_77, _78), _79);
    float _RESERVED_IDENTIFIER_FIXUP_6_mx = _81;
    float3 _84 = 0.0f.xxx;
    if (_81 > _75)
    {
        float3 _90 = _63;
        _84 = ((_71 - _75.xxx) * blend_color_saturation_Qhh3(_90)) * (1.0f / (_81 - _75));
    }
    else
    {
        _84 = 0.0f.xxx;
    }
    _RESERVED_IDENTIFIER_FIXUP_3_l = _84;
    _RESERVED_IDENTIFIER_FIXUP_4_r = _71;
    float _100 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _71);
    float _RESERVED_IDENTIFIER_FIXUP_7_lum = _100;
    float3 _109 = (_100 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _84)).xxx + _84;
    float3 _RESERVED_IDENTIFIER_FIXUP_8_result = _109;
    float _113 = _109.x;
    float _114 = _109.y;
    float _115 = _109.z;
    float _111 = min(min(_113, _114), _115);
    float _RESERVED_IDENTIFIER_FIXUP_9_minComp = _111;
    float _117 = max(max(_113, _114), _115);
    float _RESERVED_IDENTIFIER_FIXUP_10_maxComp = _117;
    bool _123 = false;
    if (_111 < 0.0f)
    {
        _123 = _100 != _111;
    }
    else
    {
        _123 = false;
    }
    if (_123)
    {
        float3 _126 = _100.xxx;
        _RESERVED_IDENTIFIER_FIXUP_8_result = _126 + ((_109 - _126) * (_100 / (((_100 - _111) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _140 = false;
    if (_117 > _55)
    {
        _140 = _117 != _100;
    }
    else
    {
        _140 = false;
    }
    if (_140)
    {
        float3 _144 = _100.xxx;
        _RESERVED_IDENTIFIER_FIXUP_8_result = _144 + (((_RESERVED_IDENTIFIER_FIXUP_8_result - _144) * (_55 - _100)) * (1.0f / (((_117 - _100) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    sk_FragColor = float4((((_RESERVED_IDENTIFIER_FIXUP_8_result + _15_dst.xyz) - _71) + _15_src.xyz) - _63, (_15_src.w + _15_dst.w) - _55);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
