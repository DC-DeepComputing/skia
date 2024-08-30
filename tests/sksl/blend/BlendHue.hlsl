cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_src : packoffset(c0);
    float4 _9_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float blend_color_saturation_Qhh3(float3 _15)
{
    return max(max(_15.x, _15.y), _15.z) - min(min(_15.x, _15.y), _15.z);
}

float4 blend_hslc_h4h2h4h4(float2 _38, float4 _39, float4 _40)
{
    float _48 = _40.w * _39.w;
    float alpha = _48;
    float3 _54 = _39.xyz * _40.w;
    float3 sda = _54;
    float3 _60 = _40.xyz * _39.w;
    float3 dsa = _60;
    float3 _67 = 0.0f.xxx;
    if (_38.x != 0.0f)
    {
        _67 = _60;
    }
    else
    {
        _67 = _54;
    }
    float3 l = _67;
    float3 _76 = 0.0f.xxx;
    if (_38.x != 0.0f)
    {
        _76 = _54;
    }
    else
    {
        _76 = _60;
    }
    float3 r = _76;
    if (_38.y != 0.0f)
    {
        float _87 = min(min(_67.x, _67.y), _67.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _87;
        float _93 = max(max(_67.x, _67.y), _67.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _93;
        float3 _96 = 0.0f.xxx;
        if (_93 > _87)
        {
            float3 _102 = _76;
            _96 = ((_67 - _87.xxx) * blend_color_saturation_Qhh3(_102)) * (1.0f / (_93 - _87));
        }
        else
        {
            _96 = 0.0f.xxx;
        }
        l = _96;
        r = _60;
    }
    float _112 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_4_lum = _112;
    float3 _124 = (_112 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_5_result = _124;
    float _128 = _124.x;
    float _129 = _124.y;
    float _130 = _124.z;
    float _126 = min(min(_128, _129), _130);
    float _RESERVED_IDENTIFIER_FIXUP_6_minComp = _126;
    float _132 = max(max(_128, _129), _130);
    float _RESERVED_IDENTIFIER_FIXUP_7_maxComp = _132;
    bool _139 = false;
    if (_126 < 0.0f)
    {
        _139 = _112 != _126;
    }
    else
    {
        _139 = false;
    }
    if (_139)
    {
        float3 _142 = _112.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _142 + ((_124 - _142) * (_112 / ((_112 - _126) + 6.103515625e-05f)));
    }
    bool _154 = false;
    if (_132 > _48)
    {
        _154 = _132 != _112;
    }
    else
    {
        _154 = false;
    }
    if (_154)
    {
        float3 _158 = _112.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _158 + (((_RESERVED_IDENTIFIER_FIXUP_5_result - _158) * (_48 - _112)) * (1.0f / ((_132 - _112) + 6.103515625e-05f)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_5_result + _40.xyz) - _60) + _39.xyz) - _54, (_39.w + _40.w) - _48);
}

void frag_main()
{
    float2 _190 = float2(0.0f, 1.0f);
    float4 _196 = _9_src;
    float4 _200 = _9_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_190, _196, _200);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
