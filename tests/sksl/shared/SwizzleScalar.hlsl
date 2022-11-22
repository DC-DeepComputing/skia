static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  float4 h4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_32 = asfloat(x_10[0].x);
  h4 = float4(x_32, x_32, x_32, x_32);
  const float x_35 = asfloat(x_10[0].x);
  const float2 x_36 = float2(x_35, x_35);
  h4 = float4(x_36.x, x_36.y, 0.0f, 1.0f);
  const float x_42 = asfloat(x_10[0].x);
  h4 = float4(0.0f, x_42, 1.0f, 0.0f);
  const float x_45 = asfloat(x_10[0].x);
  const float x_47 = asfloat(x_10[0].x);
  const float4 x_48 = float4(0.0f, x_45, 0.0f, x_47);
  h4 = x_48;
  return x_48;
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
