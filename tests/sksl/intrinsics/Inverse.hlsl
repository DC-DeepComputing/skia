static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  float2x2 inv2x2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 inv3x3 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 inv4x4 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_122 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_82 = false;
  bool x_83 = false;
  bool x_97 = false;
  bool x_98 = false;
  bool x_120 = false;
  bool x_121 = false;
  inv2x2 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
  inv3x3 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
  inv4x4 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
  x_83 = false;
  if (true) {
    x_82 = true;
    x_83 = x_82;
  }
  x_98 = false;
  if (x_83) {
    x_97 = true;
    x_98 = x_97;
  }
  x_121 = false;
  if (x_98) {
    const float s = (1.0f / determinant(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f))));
    const float3x3 x_101 = (s * float3x3(float3(-3.0f, 6.0f, -3.0f), float3(6.0f, -12.0f, 6.0f), float3(-3.0f, 6.0f, -3.0f)));
    x_120 = ((any(!((x_101[0u] == float3(-24.0f, 18.0f, 5.0f)))) | any(!((x_101[1u] == float3(20.0f, -15.0f, -4.0f))))) | any(!((x_101[2u] == float3(-5.0f, 4.0f, 1.0f)))));
    x_121 = x_120;
  }
  if (x_121) {
    const float4 x_131 = asfloat(x_10[0]);
    x_122 = x_131;
  } else {
    const float4 x_134 = asfloat(x_10[1]);
    x_122 = x_134;
  }
  const float4 x_135 = x_122;
  return x_135;
}

void _entrypoint_1() {
  float2 x_20 = float2(0.0f, 0.0f);
  x_20 = (0.0f).xx;
  const float4 x_22 = main(x_20);
  sk_FragColor = x_22;
  return;
}

struct _entrypoint_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

_entrypoint_out _entrypoint_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  _entrypoint_1();
  const _entrypoint_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
