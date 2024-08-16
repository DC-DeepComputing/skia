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

float color_dodge_component_Qhh2h2(float2 _21, float2 _22)
{
    float _29 = float(_22.x != 0.0f);
    float dxScale = _29;
    float _81 = _29 * min(_22.y, (abs(_21.y - _21.x) >= 6.103515625e-05f) ? ((_22.x * _21.y) / ((_21.y - _21.x) + _kGuardedDivideEpsilon)) : _22.y);
    float delta = _81;
    return ((_81 * _21.y) + (_21.x * (1.0f - _22.y))) + (_22.x * (1.0f - _21.y));
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _108 = _15_src.xw;
    float2 _113 = _15_dst.xw;
    float2 _118 = _15_src.yw;
    float2 _122 = _15_dst.yw;
    float2 _127 = _15_src.zw;
    float2 _131 = _15_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_108, _113), color_dodge_component_Qhh2h2(_118, _122), color_dodge_component_Qhh2h2(_127, _131), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
