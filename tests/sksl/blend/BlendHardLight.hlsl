cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_src : packoffset(c0);
    float4 _8_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 blend_overlay_h4h4h4(float4 _13, float4 _14)
{
    float _25 = 0.0f;
    if ((2.0f * _14.x) <= _14.w)
    {
        _25 = (2.0f * _13.x) * _14.x;
    }
    else
    {
        _25 = (_13.w * _14.w) - ((2.0f * (_14.w - _14.x)) * (_13.w - _13.x));
    }
    float _61 = 0.0f;
    if ((2.0f * _14.y) <= _14.w)
    {
        _61 = (2.0f * _13.y) * _14.y;
    }
    else
    {
        _61 = (_13.w * _14.w) - ((2.0f * (_14.w - _14.y)) * (_13.w - _13.y));
    }
    float _96 = 0.0f;
    if ((2.0f * _14.z) <= _14.w)
    {
        _96 = (2.0f * _13.z) * _14.z;
    }
    else
    {
        _96 = (_13.w * _14.w) - ((2.0f * (_14.w - _14.z)) * (_13.w - _13.z));
    }
    float4 result = float4(_25, _61, _96, _13.w + ((1.0f - _13.w) * _14.w));
    float4 _136 = result;
    float3 _152 = _136.xyz + ((_14.xyz * (1.0f - _13.w)) + (_13.xyz * (1.0f - _14.w)));
    float4 _153 = result;
    float4 _154 = float4(_152.x, _152.y, _152.z, _153.w);
    result = _154;
    return _154;
}

void frag_main()
{
    float4 _163 = _8_dst;
    float4 _167 = _8_src;
    sk_FragColor = blend_overlay_h4h4h4(_163, _167);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
