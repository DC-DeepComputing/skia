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
    float _32 = _14_dst.w * _14_src.w;
    float _RESERVED_IDENTIFIER_FIXUP_0_alpha = _32;
    float3 _42 = _14_src.xyz * _14_dst.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_sda = _42;
    float3 _50 = _14_dst.xyz * _14_src.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_2_dsa = _50;
    float3 _RESERVED_IDENTIFIER_FIXUP_3_l = _42;
    float3 _RESERVED_IDENTIFIER_FIXUP_4_r = _50;
    float _54 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _50);
    float _RESERVED_IDENTIFIER_FIXUP_5_lum = _54;
    float3 _63 = (_54 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _42)).xxx + _42;
    float3 _RESERVED_IDENTIFIER_FIXUP_6_result = _63;
    float _67 = _63.x;
    float _68 = _63.y;
    float _69 = _63.z;
    float _65 = min(min(_67, _68), _69);
    float _RESERVED_IDENTIFIER_FIXUP_7_minComp = _65;
    float _71 = max(max(_67, _68), _69);
    float _RESERVED_IDENTIFIER_FIXUP_8_maxComp = _71;
    bool _77 = false;
    if (_65 < 0.0f)
    {
        _77 = _54 != _65;
    }
    else
    {
        _77 = false;
    }
    if (_77)
    {
        float3 _80 = _54.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _80 + ((_63 - _80) * (_54 / (((_54 - _65) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _94 = false;
    if (_71 > _32)
    {
        _94 = _71 != _54;
    }
    else
    {
        _94 = false;
    }
    if (_94)
    {
        float3 _98 = _54.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _98 + (((_RESERVED_IDENTIFIER_FIXUP_6_result - _98) * (_32 - _54)) * (1.0f / (((_71 - _54) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    sk_FragColor = float4((((_RESERVED_IDENTIFIER_FIXUP_6_result + _14_dst.xyz) - _50) + _14_src.xyz) - _42, (_14_src.w + _14_dst.w) - _32);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
