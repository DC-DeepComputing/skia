cbuffer _UniformBuffer : register(b0, space0)
{
    int _11_a : packoffset(c0);
    uint _11_b : packoffset(c0.y);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor.x = float(countbits(_11_a)) + float(int(countbits(_11_b)));
    int2 _33 = countbits(_11_a.xx);
    int2 _44 = int2(countbits(_11_b.xx));
    float2 _54 = float2(float(_33.x), float(_33.y)) + float2(float(_44.x), float(_44.y));
    sk_FragColor = float4(_54.x, _54.y, sk_FragColor.z, sk_FragColor.w);
    int3 _57 = countbits(_11_a.xxx);
    int3 _70 = int3(countbits(_11_b.xxx));
    float3 _82 = float3(float(_57.x), float(_57.y), float(_57.z)) + float3(float(_70.x), float(_70.y), float(_70.z));
    sk_FragColor = float4(_82.x, _82.y, _82.z, sk_FragColor.w);
    int4 _85 = countbits(_11_a.xxxx);
    int4 _99 = int4(countbits(_11_b.xxxx));
    sk_FragColor = float4(float(_85.x), float(_85.y), float(_85.z), float(_85.w)) + float4(float(_99.x), float(_99.y), float(_99.z), float(_99.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
