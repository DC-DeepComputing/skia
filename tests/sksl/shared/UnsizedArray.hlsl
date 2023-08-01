struct ElementWithArrayMember
{
    float array[2];
};

struct ElementWithNoArray
{
    float2 a;
};

RWByteAddressBuffer _3 : register(u0, space0);
RWByteAddressBuffer _12 : register(u1, space0);
RWByteAddressBuffer _17 : register(u2, space0);
RWByteAddressBuffer _20 : register(u3, space0);

static uint3 gl_GlobalInvocationID;
struct SPIRV_Cross_Input
{
    uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

void comp_main()
{
    uint idx = gl_GlobalInvocationID.x;
    ElementWithArrayMember _41 = { { 0.0f, 0.0f } };
    [unroll]
    for (int _4ident = 0; _4ident < 2; _4ident++)
    {
        _41.array[_4ident] = asfloat(_3.Load(_4ident * 16 + gl_GlobalInvocationID.x * 32 + 0));
    }
    ElementWithArrayMember _RESERVED_IDENTIFIER_FIXUP_0_e = _41;
    float _49 = _RESERVED_IDENTIFIER_FIXUP_0_e.array[0] * _RESERVED_IDENTIFIER_FIXUP_0_e.array[1];
    float f = _49;
    float _54[2] = { 0.0f, 0.0f };
    [unroll]
    for (int _5ident = 0; _5ident < 2; _5ident++)
    {
        [unroll]
        for (int _6ident = 0; _6ident < 2; _6ident++)
        {
            _54[_5ident][_6ident] = asfloat(_3.Load(_6ident * 16 + _5ident * 16 + gl_GlobalInvocationID.x * 32 + 0));
        }
    }
    float _RESERVED_IDENTIFIER_FIXUP_1_arr[2] = _54;
    float _60 = _49 * (_RESERVED_IDENTIFIER_FIXUP_1_arr[0] * _RESERVED_IDENTIFIER_FIXUP_1_arr[1]);
    f = _60;
    ElementWithArrayMember e = { { 0.0f, 0.0f } };
    e.array[0] = _60 * asfloat(_12.Load(gl_GlobalInvocationID.x * 4 + 4));
    e.array[1] = f * asfloat(_12.Load((idx + 1u) * 4 + 4));
    [unroll]
    for (int _7ident = 0; _7ident < 2; _7ident++)
    {
        _17.Store(_7ident * 16 + idx * 32 + 0, asuint(e.array[_7ident]));
    }
}

[numthreads(16, 16, 1)]
void main(SPIRV_Cross_Input stage_input)
{
    gl_GlobalInvocationID = stage_input.gl_GlobalInvocationID;
    comp_main();
}
