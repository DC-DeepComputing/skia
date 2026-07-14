cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _21_src : packoffset(c0);
    float4 _21_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float blend_color_saturation_Qhh3(float3 _27)
{
    return max(max(_27.x, _27.y), _27.z) - min(min(_27.x, _27.y), _27.z);
}

float3 guarded_divide_Qh3h3h(float3 _48, float _49)
{
    return _48 * (1.0f / (_49 + _kGuardedDivideEpsilon));
}

float4 blend_hslc_h4h2h4h4(float2 _62, float4 _63, float4 _64)
{
    float _71 = _64.w * _63.w;
    float alpha = _71;
    float3 _77 = _63.xyz * _64.w;
    float3 sda = _77;
    float3 _83 = _64.xyz * _63.w;
    float3 dsa = _83;
    float3 _88 = 0.0f.xxx;
    if (_62.x != 0.0f)
    {
        _88 = _83;
    }
    else
    {
        _88 = _77;
    }
    float3 l = _88;
    float3 _97 = 0.0f.xxx;
    if (_62.x != 0.0f)
    {
        _97 = _77;
    }
    else
    {
        _97 = _83;
    }
    float3 r = _97;
    if (_62.y != 0.0f)
    {
        float _108 = min(min(_88.x, _88.y), _88.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _108;
        float _114 = max(max(_88.x, _88.y), _88.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _114;
        float _117 = _114 - _108;
        float _RESERVED_IDENTIFIER_FIXUP_4_diff = _117;
        float3 _119 = 0.0f.xxx;
        if (_117 > 0.0f)
        {
            float3 _125 = _97;
            float3 _128 = (_88 - _108.xxx) * blend_color_saturation_Qhh3(_125);
            float _129 = _117;
            _119 = guarded_divide_Qh3h3h(_128, _129);
        }
        else
        {
            _119 = 0.0f.xxx;
        }
        l = _119;
        r = _83;
    }
    float _134 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_5_lum = _134;
    float3 _146 = (_134 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_6_result = _146;
    float _150 = _146.x;
    float _151 = _146.y;
    float _152 = _146.z;
    float _148 = min(min(_150, _151), _152);
    float _RESERVED_IDENTIFIER_FIXUP_7_minComp = _148;
    float _154 = max(max(_150, _151), _152);
    float _RESERVED_IDENTIFIER_FIXUP_8_maxComp = _154;
    bool _160 = false;
    if (_148 < 0.0f)
    {
        _160 = _134 != _148;
    }
    else
    {
        _160 = false;
    }
    if (_160)
    {
        float3 _163 = _134.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _163 + ((_146 - _163) * (_134 / (((_134 - _148) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _177 = false;
    if (_154 > _71)
    {
        _177 = _154 != _134;
    }
    else
    {
        _177 = false;
    }
    if (_177)
    {
        float3 _181 = _134.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _181 + (((_RESERVED_IDENTIFIER_FIXUP_6_result - _181) * (_71 - _134)) * (1.0f / (((_154 - _134) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_6_result + _64.xyz) - _83) + _63.xyz) - _77, (_63.w + _64.w) - _71);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _215 = float2(1.0f, 0.0f);
    float4 _220 = _21_src;
    float4 _224 = _21_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_215, _220, _224);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
