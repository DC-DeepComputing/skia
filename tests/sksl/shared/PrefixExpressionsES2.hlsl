cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    bool ok = true;
    int i = 5;
    int _33 = 5 + 1;
    i = _33;
    bool _39 = false;
    if (true)
    {
        _39 = _33 == 6;
    }
    else
    {
        _39 = false;
    }
    ok = _39;
    bool _45 = false;
    if (_39)
    {
        int _42 = _33 + 1;
        i = _42;
        _45 = _42 == 7;
    }
    else
    {
        _45 = false;
    }
    ok = _45;
    bool _51 = false;
    if (_45)
    {
        int _48 = i;
        int _49 = _48 - 1;
        i = _49;
        _51 = _49 == 6;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    int _52 = i;
    int _53 = _52 - 1;
    i = _53;
    bool _57 = false;
    if (_51)
    {
        _57 = _53 == 5;
    }
    else
    {
        _57 = false;
    }
    ok = _57;
    float f = 0.5f;
    float _62 = 0.5f + 1.0f;
    f = _62;
    bool _67 = false;
    if (_57)
    {
        _67 = _62 == 1.5f;
    }
    else
    {
        _67 = false;
    }
    ok = _67;
    bool _73 = false;
    if (_67)
    {
        float _70 = _62 + 1.0f;
        f = _70;
        _73 = _70 == 2.5f;
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _79 = false;
    if (_73)
    {
        float _76 = f;
        float _77 = _76 - 1.0f;
        f = _77;
        _79 = _77 == 1.5f;
    }
    else
    {
        _79 = false;
    }
    ok = _79;
    float _80 = f;
    float _81 = _80 - 1.0f;
    f = _81;
    bool _85 = false;
    if (_79)
    {
        _85 = _81 == 0.5f;
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    float2 f2 = 0.5f.xx;
    f2.x += 1.0f;
    bool _98 = false;
    if (ok)
    {
        _98 = f2.x == 1.5f;
    }
    else
    {
        _98 = false;
    }
    ok = _98;
    bool _105 = false;
    if (_98)
    {
        float _102 = f2.x;
        float _103 = _102 + 1.0f;
        f2.x = _103;
        _105 = _103 == 2.5f;
    }
    else
    {
        _105 = false;
    }
    ok = _105;
    bool _112 = false;
    if (_105)
    {
        float _109 = f2.x;
        float _110 = _109 - 1.0f;
        f2.x = _110;
        _112 = _110 == 1.5f;
    }
    else
    {
        _112 = false;
    }
    ok = _112;
    f2.x -= 1.0f;
    bool _122 = false;
    if (ok)
    {
        _122 = f2.x == 0.5f;
    }
    else
    {
        _122 = false;
    }
    ok = _122;
    float2 _124 = f2;
    float2 _125 = _124 + 1.0f.xx;
    f2 = _125;
    bool _132 = false;
    if (_122)
    {
        _132 = all(bool2(_125.x == 1.5f.xx.x, _125.y == 1.5f.xx.y));
    }
    else
    {
        _132 = false;
    }
    ok = _132;
    bool _139 = false;
    if (_132)
    {
        float2 _135 = _125 + 1.0f.xx;
        f2 = _135;
        _139 = all(bool2(_135.x == 2.5f.xx.x, _135.y == 2.5f.xx.y));
    }
    else
    {
        _139 = false;
    }
    ok = _139;
    bool _146 = false;
    if (_139)
    {
        float2 _142 = f2;
        float2 _143 = _142 - 1.0f.xx;
        f2 = _143;
        _146 = all(bool2(_143.x == 1.5f.xx.x, _143.y == 1.5f.xx.y));
    }
    else
    {
        _146 = false;
    }
    ok = _146;
    float2 _147 = f2;
    float2 _148 = _147 - 1.0f.xx;
    f2 = _148;
    bool _153 = false;
    if (_146)
    {
        _153 = all(bool2(_148.x == 0.5f.xx.x, _148.y == 0.5f.xx.y));
    }
    else
    {
        _153 = false;
    }
    ok = _153;
    int4 i4 = int4(7, 8, 9, 10);
    int4 _162 = int4(7, 8, 9, 10) + int4(1, 1, 1, 1);
    i4 = _162;
    bool _170 = false;
    if (_153)
    {
        _170 = all(bool4(_162.x == int4(8, 9, 10, 11).x, _162.y == int4(8, 9, 10, 11).y, _162.z == int4(8, 9, 10, 11).z, _162.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _170 = false;
    }
    ok = _170;
    bool _178 = false;
    if (_170)
    {
        int4 _173 = _162 + int4(1, 1, 1, 1);
        i4 = _173;
        _178 = all(bool4(_173.x == int4(9, 10, 11, 12).x, _173.y == int4(9, 10, 11, 12).y, _173.z == int4(9, 10, 11, 12).z, _173.w == int4(9, 10, 11, 12).w));
    }
    else
    {
        _178 = false;
    }
    ok = _178;
    bool _185 = false;
    if (_178)
    {
        int4 _181 = i4;
        int4 _182 = _181 - int4(1, 1, 1, 1);
        i4 = _182;
        _185 = all(bool4(_182.x == int4(8, 9, 10, 11).x, _182.y == int4(8, 9, 10, 11).y, _182.z == int4(8, 9, 10, 11).z, _182.w == int4(8, 9, 10, 11).w));
    }
    else
    {
        _185 = false;
    }
    ok = _185;
    int4 _186 = i4;
    int4 _187 = _186 - int4(1, 1, 1, 1);
    i4 = _187;
    bool _192 = false;
    if (_185)
    {
        _192 = all(bool4(_187.x == int4(7, 8, 9, 10).x, _187.y == int4(7, 8, 9, 10).y, _187.z == int4(7, 8, 9, 10).z, _187.w == int4(7, 8, 9, 10).w));
    }
    else
    {
        _192 = false;
    }
    ok = _192;
    float2x3 m2x3 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
    float3 _207 = float3(1.0f, 2.0f, 3.0f) + 1.0f.xxx;
    float3 _208 = float3(4.0f, 5.0f, 6.0f) + 1.0f.xxx;
    m2x3 = float2x3(_207, _208);
    bool _222 = false;
    if (_192)
    {
        _222 = all(bool3(_207.x == float3(2.0f, 3.0f, 4.0f).x, _207.y == float3(2.0f, 3.0f, 4.0f).y, _207.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_208.x == float3(5.0f, 6.0f, 7.0f).x, _208.y == float3(5.0f, 6.0f, 7.0f).y, _208.z == float3(5.0f, 6.0f, 7.0f).z));
    }
    else
    {
        _222 = false;
    }
    ok = _222;
    bool _237 = false;
    if (_222)
    {
        float3 _225 = _207 + 1.0f.xxx;
        float3 _226 = _208 + 1.0f.xxx;
        m2x3 = float2x3(_225, _226);
        _237 = all(bool3(_225.x == float3(3.0f, 4.0f, 5.0f).x, _225.y == float3(3.0f, 4.0f, 5.0f).y, _225.z == float3(3.0f, 4.0f, 5.0f).z)) && all(bool3(_226.x == float3(6.0f, 7.0f, 8.0f).x, _226.y == float3(6.0f, 7.0f, 8.0f).y, _226.z == float3(6.0f, 7.0f, 8.0f).z));
    }
    else
    {
        _237 = false;
    }
    ok = _237;
    bool _251 = false;
    if (_237)
    {
        float2x3 _240 = m2x3;
        float3 _242 = _240[0] - 1.0f.xxx;
        float3 _244 = _240[1] - 1.0f.xxx;
        m2x3 = float2x3(_242, _244);
        _251 = all(bool3(_242.x == float3(2.0f, 3.0f, 4.0f).x, _242.y == float3(2.0f, 3.0f, 4.0f).y, _242.z == float3(2.0f, 3.0f, 4.0f).z)) && all(bool3(_244.x == float3(5.0f, 6.0f, 7.0f).x, _244.y == float3(5.0f, 6.0f, 7.0f).y, _244.z == float3(5.0f, 6.0f, 7.0f).z));
    }
    else
    {
        _251 = false;
    }
    ok = _251;
    float2x3 _252 = m2x3;
    float3 _254 = _252[0] - 1.0f.xxx;
    float3 _256 = _252[1] - 1.0f.xxx;
    m2x3 = float2x3(_254, _256);
    bool _265 = false;
    if (_251)
    {
        _265 = all(bool3(_254.x == float3(1.0f, 2.0f, 3.0f).x, _254.y == float3(1.0f, 2.0f, 3.0f).y, _254.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_256.x == float3(4.0f, 5.0f, 6.0f).x, _256.y == float3(4.0f, 5.0f, 6.0f).y, _256.z == float3(4.0f, 5.0f, 6.0f).z));
    }
    else
    {
        _265 = false;
    }
    ok = _265;
    bool _273 = false;
    if (_265)
    {
        _273 = _7_colorGreen.x != 1.0f;
    }
    else
    {
        _273 = false;
    }
    ok = _273;
    bool _282 = false;
    if (_273)
    {
        _282 = (-1.0f) == (-_7_colorGreen.y);
    }
    else
    {
        _282 = false;
    }
    ok = _282;
    bool _291 = false;
    if (_282)
    {
        float4 _288 = -_7_colorGreen;
        _291 = all(bool4(float4(0.0f, -1.0f, 0.0f, -1.0f).x == _288.x, float4(0.0f, -1.0f, 0.0f, -1.0f).y == _288.y, float4(0.0f, -1.0f, 0.0f, -1.0f).z == _288.z, float4(0.0f, -1.0f, 0.0f, -1.0f).w == _288.w));
    }
    else
    {
        _291 = false;
    }
    ok = _291;
    bool _314 = false;
    if (_291)
    {
        float2 _305 = -_7_testMatrix2x2[0];
        float2 _307 = -_7_testMatrix2x2[1];
        _314 = all(bool2(float2(-1.0f, -2.0f).x == _305.x, float2(-1.0f, -2.0f).y == _305.y)) && all(bool2(float2(-3.0f, -4.0f).x == _307.x, float2(-3.0f, -4.0f).y == _307.y));
    }
    else
    {
        _314 = false;
    }
    ok = _314;
    int2 _321 = int2(i, -i);
    int2 iv = _321;
    bool _328 = false;
    if (_314)
    {
        _328 = (-i) == (-5);
    }
    else
    {
        _328 = false;
    }
    ok = _328;
    bool _335 = false;
    if (_328)
    {
        int2 _331 = -_321;
        _335 = all(bool2(_331.x == int2(-5, 5).x, _331.y == int2(-5, 5).y));
    }
    else
    {
        _335 = false;
    }
    ok = _335;
    float4 _336 = 0.0f.xxxx;
    if (_335)
    {
        _336 = _7_colorGreen;
    }
    else
    {
        _336 = _7_colorRed;
    }
    return _336;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
