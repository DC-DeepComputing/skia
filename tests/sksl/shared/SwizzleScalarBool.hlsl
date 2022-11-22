static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  bool b = false;
  bool4 b4 = bool4(false, false, false, false);
  const float x_32 = asfloat(x_10[0].x);
  const bool x_33 = !((x_32 == 0.0f));
  b = x_33;
  b4 = bool4(x_33, x_33, x_33, x_33);
  const bool2 x_39 = bool2(x_33, x_33);
  b4 = bool4(x_39.x, x_39.y, false, true);
  b4 = bool4(false, x_33, true, false);
  const bool4 x_46 = bool4(false, x_33, false, x_33);
  b4 = x_46;
  return float4(0.0f, (x_46.y ? 1.0f : 0.0f), (x_46.z ? 1.0f : 0.0f), (x_46.w ? 1.0f : 0.0f));
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
