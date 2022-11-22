static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  float result = 0.0f;
  float2x2 a = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 b = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 c = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 d = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 e = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 f = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_101 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  result = 0.0f;
  a = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
  const float2 x_44 = a[0];
  const float x_46 = (0.0f + x_44.x);
  result = x_46;
  b = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
  const float2 x_58 = b[0];
  const float x_60 = (x_46 + x_58.x);
  result = x_60;
  c = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
  const float3 x_69 = c[0];
  const float x_71 = (x_60 + x_69.x);
  result = x_71;
  d = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
  const float3 x_77 = d[0];
  const float x_79 = (x_71 + x_77.x);
  result = x_79;
  e = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
  const float4 x_84 = e[0];
  const float x_86 = (x_79 + x_84.x);
  result = x_86;
  const float3 x_88 = float3(1.0f, 0.0f, 0.0f);
  const float3 x_89 = float3(0.0f, 1.0f, 0.0f);
  const float3x3 x_91 = float3x3(x_88, x_89, float3(0.0f, 0.0f, 1.0f));
  f = float2x2(float2(x_88.x, x_88.y), float2(x_89.x, x_89.y));
  const float2 x_96 = f[0];
  const float x_98 = (x_86 + x_96.x);
  result = x_98;
  if ((x_98 == 6.0f)) {
    const float4 x_107 = asfloat(x_10[0]);
    x_101 = x_107;
  } else {
    const float4 x_110 = asfloat(x_10[1]);
    x_101 = x_110;
  }
  const float4 x_111 = x_101;
  return x_111;
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
