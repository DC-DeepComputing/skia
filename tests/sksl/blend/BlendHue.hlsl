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
    float _56 = _42.x;
    float _57 = _42.y;
    float _58 = _42.z;
    float _54 = min(min(_56, _57), _58);
    float _RESERVED_IDENTIFIER_FIXUP_5_mn = _54;
    float _60 = max(max(_56, _57), _58);
    float _RESERVED_IDENTIFIER_FIXUP_6_mx = _60;
    float3 _63 = 0.0f.xxx;
    if (_60 > _54)
    {
        float _71 = _50.x;
        float _72 = _50.y;
        float _73 = _50.z;
        _63 = ((_42 - _54.xxx) * (max(max(_71, _72), _73) - min(min(_71, _72), _73))) * (1.0f / (_60 - _54));
    }
    else
    {
        _63 = 0.0f.xxx;
    }
    _RESERVED_IDENTIFIER_FIXUP_3_l = _63;
    _RESERVED_IDENTIFIER_FIXUP_4_r = _50;
    float _85 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _50);
    float _RESERVED_IDENTIFIER_FIXUP_7_lum = _85;
    float3 _94 = (_85 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _63)).xxx + _63;
    float3 _RESERVED_IDENTIFIER_FIXUP_8_result = _94;
    float _98 = _94.x;
    float _99 = _94.y;
    float _100 = _94.z;
    float _96 = min(min(_98, _99), _100);
    float _RESERVED_IDENTIFIER_FIXUP_9_minComp = _96;
    float _102 = max(max(_98, _99), _100);
    float _RESERVED_IDENTIFIER_FIXUP_10_maxComp = _102;
    bool _108 = false;
    if (_96 < 0.0f)
    {
        _108 = _85 != _96;
    }
    else
    {
        _108 = false;
    }
    if (_108)
    {
        float3 _111 = _85.xxx;
        _RESERVED_IDENTIFIER_FIXUP_8_result = _111 + ((_94 - _111) * (_85 / (((_85 - _96) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _125 = false;
    if (_102 > _32)
    {
        _125 = _102 != _85;
    }
    else
    {
        _125 = false;
    }
    if (_125)
    {
        float3 _129 = _85.xxx;
        _RESERVED_IDENTIFIER_FIXUP_8_result = _129 + (((_RESERVED_IDENTIFIER_FIXUP_8_result - _129) * (_32 - _85)) * (1.0f / (((_102 - _85) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    sk_FragColor = float4((((_RESERVED_IDENTIFIER_FIXUP_8_result + _14_dst.xyz) - _50) + _14_src.xyz) - _42, (_14_src.w + _14_dst.w) - _32);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
