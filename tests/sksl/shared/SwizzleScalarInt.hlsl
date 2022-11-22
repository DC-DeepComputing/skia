static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  int i = 0;
  int4 i4 = int4(0, 0, 0, 0);
  const float x_32 = asfloat(x_10[0].x);
  const int x_33 = int(x_32);
  i = x_33;
  i4 = int4(x_33, x_33, x_33, x_33);
  const int2 x_39 = int2(x_33, x_33);
  i4 = int4(x_39.x, x_39.y, 0, 1);
  i4 = int4(0, x_33, 1, 0);
  const int4 x_45 = int4(0, x_33, 0, x_33);
  i4 = x_45;
  return float4(0.0f, float(x_45.y), float(x_45.z), float(x_45.w));
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
