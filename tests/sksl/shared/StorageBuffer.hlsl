struct SomeData
{
    float4 a;
    float2 b;
};

RWByteAddressBuffer _3 : register(u0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _27)
{
    return asfloat(_3.Load4(_3.Load(0) * 32 + 16)) * asfloat(_3.Load2(_3.Load(0) * 32 + 32)).x;
}

void frag_main()
{
    float2 _23 = 0.0f.xx;
    sk_FragColor = main(_23);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
