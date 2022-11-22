static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  float f = 0.0f;
  int i = 0;
  bool b = false;
  float f1 = 0.0f;
  float f2 = 0.0f;
  float f3 = 0.0f;
  int i1 = 0;
  int i2 = 0;
  int i3 = 0;
  bool b1 = false;
  bool b2 = false;
  bool b3 = false;
  float4 x_79 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_32 = asfloat(x_10[0]);
  const float x_33 = x_32.y;
  f = x_33;
  const float4 x_37 = asfloat(x_10[0]);
  const int x_39 = int(x_37.y);
  i = x_39;
  const float4 x_43 = asfloat(x_10[0]);
  const bool x_45 = !((x_43.y == 0.0f));
  b = x_45;
  f1 = x_33;
  const float x_48 = float(x_39);
  f2 = x_48;
  const float x_50 = (x_45 ? 1.0f : 0.0f);
  f3 = x_50;
  const int x_53 = int(x_33);
  i1 = x_53;
  i2 = x_39;
  const int x_56 = (x_45 ? 1 : 0);
  i3 = x_56;
  const bool x_59 = !((x_33 == 0.0f));
  b1 = x_59;
  const bool x_61 = (x_39 != 0);
  b2 = x_61;
  b3 = x_45;
  if ((((((((((x_33 + x_48) + x_50) + float(x_53)) + float(x_39)) + float(x_56)) + (x_59 ? 1.0f : 0.0f)) + (x_61 ? 1.0f : 0.0f)) + (x_45 ? 1.0f : 0.0f)) == 9.0f)) {
    const float4 x_85 = asfloat(x_10[0]);
    x_79 = x_85;
  } else {
    const float4 x_87 = asfloat(x_10[1]);
    x_79 = x_87;
  }
  const float4 x_88 = x_79;
  return x_88;
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
