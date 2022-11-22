static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  float x_0_x = 0.0f;
  float2 x_1_x = float2(0.0f, 0.0f);
  x_0_x = 1.0f;
  const float x_29 = length(1.0f);
  x_0_x = x_29;
  const float x_30 = distance(x_29, 2.0f);
  x_0_x = x_30;
  const float x_32 = (x_30 * 2.0f);
  x_0_x = x_32;
  x_0_x = 1.0f;
  x_1_x = float2(1.0f, 2.0f);
  const float x_36 = length(float2(1.0f, 2.0f));
  const float2 x_37 = float2(x_36, x_36);
  x_1_x = x_37;
  const float x_38 = distance(x_37, float2(3.0f, 4.0f));
  const float2 x_42 = float2(x_38, x_38);
  x_1_x = x_42;
  const float x_43 = dot(x_42, float2(3.0f, 4.0f));
  const float2 x_44 = float2(x_43, x_43);
  x_1_x = x_44;
  x_1_x = normalize(x_44);
  const float4 x_50 = asfloat(x_10[0]);
  return x_50;
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
