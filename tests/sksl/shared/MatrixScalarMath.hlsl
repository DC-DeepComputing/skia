cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _9_colorGreen : packoffset(c0);
    float4 _9_colorRed : packoffset(c1);
    float4 _9_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_bifffff22(int _29, float _30, float _31, float _32, float _33, float2x2 _34)
{
    float one = _9_colorRed.x;
    float2 _51 = float2(_30 * _9_colorRed.x, _31 * _9_colorRed.x);
    float2 _52 = float2(_32 * _9_colorRed.x, _33 * _9_colorRed.x);
    float2x2 m2 = float2x2(_51, _52);
    switch (_29)
    {
        case 1:
        {
            m2 = float2x2(1.0f.xx + _51, 1.0f.xx + _52);
            break;
        }
        case 2:
        {
            m2 = float2x2(m2[0] - 1.0f.xx, m2[1] - 1.0f.xx);
            break;
        }
        case 3:
        {
            m2 = m2 * 2.0f;
            break;
        }
        case 4:
        {
            m2 = m2 * 0.5f;
            break;
        }
    }
    bool _96 = false;
    if (m2[0].x == _34[0].x)
    {
        _96 = m2[0].y == _34[0].y;
    }
    else
    {
        _96 = false;
    }
    bool _106 = false;
    if (_96)
    {
        _106 = m2[1].x == _34[1].x;
    }
    else
    {
        _106 = false;
    }
    bool _116 = false;
    if (_106)
    {
        _116 = m2[1].y == _34[1].y;
    }
    else
    {
        _116 = false;
    }
    return _116;
}

bool divisionTest_b()
{
    float _124 = _9_colorRed.x * 10.0f;
    float ten = _124;
    float2 _126 = _124.xx;
    float2x2 _127 = float2x2(_126, _126);
    float2x2 mat = _127;
    float2x2 _134 = _127 * (1.0f / _9_testInputs.x);
    float2x2 div = _134;
    float2x2 _139 = _127 * (1.0f / _9_testInputs.x);
    mat = _139;
    float4 _142 = abs(float4(_134[0].x, _134[0].y, _134[1].x, _134[1].y) + 8.0f.xxxx);
    bool _165 = false;
    if (all(bool4(_142.x < 0.00999999977648258209228515625f.xxxx.x, _142.y < 0.00999999977648258209228515625f.xxxx.y, _142.z < 0.00999999977648258209228515625f.xxxx.z, _142.w < 0.00999999977648258209228515625f.xxxx.w)))
    {
        float4 _158 = abs(float4(_139[0].x, _139[0].y, _139[1].x, _139[1].y) + 8.0f.xxxx);
        _165 = all(bool4(_158.x < 0.00999999977648258209228515625f.xxxx.x, _158.y < 0.00999999977648258209228515625f.xxxx.y, _158.z < 0.00999999977648258209228515625f.xxxx.z, _158.w < 0.00999999977648258209228515625f.xxxx.w));
    }
    else
    {
        _165 = false;
    }
    return _165;
}

float4 main(float2 _167)
{
    float f1 = _9_colorGreen.y;
    float _177 = 2.0f * _9_colorGreen.y;
    float f2 = _177;
    float _183 = 3.0f * _9_colorGreen.y;
    float f3 = _183;
    float _189 = 4.0f * _9_colorGreen.y;
    float f4 = _189;
    int _190 = 1;
    float _191 = _9_colorGreen.y;
    float _192 = _177;
    float _193 = _183;
    float _194 = _189;
    float2x2 _202 = float2x2(float2(_9_colorGreen.y + 1.0f, _177 + 1.0f), float2(_183 + 1.0f, _189 + 1.0f));
    bool _220 = false;
    if (test_bifffff22(_190, _191, _192, _193, _194, _202))
    {
        int _206 = 2;
        float _207 = _9_colorGreen.y;
        float _208 = _177;
        float _209 = _183;
        float _210 = _189;
        float2x2 _218 = float2x2(float2(_9_colorGreen.y - 1.0f, _177 - 1.0f), float2(_183 - 1.0f, _189 - 1.0f));
        _220 = test_bifffff22(_206, _207, _208, _209, _210, _218);
    }
    else
    {
        _220 = false;
    }
    bool _238 = false;
    if (_220)
    {
        int _224 = 3;
        float _225 = _9_colorGreen.y;
        float _226 = _177;
        float _227 = _183;
        float _228 = _189;
        float2x2 _236 = float2x2(float2(_9_colorGreen.y * 2.0f, _177 * 2.0f), float2(_183 * 2.0f, _189 * 2.0f));
        _238 = test_bifffff22(_224, _225, _226, _227, _228, _236);
    }
    else
    {
        _238 = false;
    }
    bool _256 = false;
    if (_238)
    {
        int _242 = 4;
        float _243 = _9_colorGreen.y;
        float _244 = _177;
        float _245 = _183;
        float _246 = _189;
        float2x2 _254 = float2x2(float2(_9_colorGreen.y * 0.5f, _177 * 0.5f), float2(_183 * 0.5f, _189 * 0.5f));
        _256 = test_bifffff22(_242, _243, _244, _245, _246, _254);
    }
    else
    {
        _256 = false;
    }
    bool _260 = false;
    if (_256)
    {
        _260 = divisionTest_b();
    }
    else
    {
        _260 = false;
    }
    float4 _261 = 0.0f.xxxx;
    if (_260)
    {
        _261 = _9_colorGreen;
    }
    else
    {
        _261 = _9_colorRed;
    }
    return _261;
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
