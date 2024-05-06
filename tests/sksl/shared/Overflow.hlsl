cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _29 = ((9.9999996169031624536541560020822e+35f * 9.9999996169031624536541560020822e+35f) * 9.9999996169031624536541560020822e+35f) * 999999988484154753734934528.0f;
    float hugeH = _29;
    float _33 = ((9.9999996169031624536541560020822e+35f * 9.9999996169031624536541560020822e+35f) * 9.9999996169031624536541560020822e+35f) * 999999988484154753734934528.0f;
    float hugeF = _33;
    int _39 = 1073741824 * 1048576;
    int hugeI = _39;
    uint _45 = 2147483648u * 524288u;
    uint hugeU = _45;
    int _50 = (16384 * 16384) * 8;
    int hugeS = _50;
    uint _55 = (32768u * 32768u) * 2u;
    uint hugeUS = _55;
    int _59 = int(0x80000000) * 524288;
    int hugeNI = _59;
    int _64 = ((-32768) * 16384) * 4;
    int hugeNS = _64;
    int4 _71 = int4(1073741824, 1073741824, 1073741824, 1073741824) * int4(32768, 32768, 32768, 32768);
    int4 hugeIvec = _71;
    uint4 _78 = uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * uint4(16384u, 16384u, 16384u, 16384u);
    uint4 hugeUvec = _78;
    float4x4 hugeMxM = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 _88 = mul(100000002004087734272.0f.xxxx, float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 hugeMxV = _88;
    float4 _90 = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), 100000002004087734272.0f.xxxx);
    float4 hugeVxM = _90;
    return ((((((((((((_7_colorGreen * clamp(_29, 0.0f, 1.0f)) * clamp(_33, 0.0f, 1.0f)) * clamp(float(_39), 0.0f, 1.0f)) * clamp(float(_45), 0.0f, 1.0f)) * clamp(float(_50), 0.0f, 1.0f)) * clamp(float(_55), 0.0f, 1.0f)) * clamp(float(_59), 0.0f, 1.0f)) * clamp(float(_64), 0.0f, 1.0f)) * clamp(float4(float(_71.x), float(_71.y), float(_71.z), float(_71.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(_78.x), float(_78.y), float(_78.z), float(_78.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(hugeMxM[0], 0.0f.xxxx, 1.0f.xxxx)) * clamp(_88, 0.0f.xxxx, 1.0f.xxxx)) * clamp(_90, 0.0f.xxxx, 1.0f.xxxx);
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
