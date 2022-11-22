static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  float f = 0.0f;
  int i = 0;
  uint u = 0u;
  bool b = false;
  float f1 = 0.0f;
  float f2 = 0.0f;
  float f3 = 0.0f;
  float f4 = 0.0f;
  int i1 = 0;
  int i2 = 0;
  int i3 = 0;
  int i4 = 0;
  uint u1 = 0u;
  uint u2 = 0u;
  uint u3 = 0u;
  uint u4 = 0u;
  bool b1 = false;
  bool b2 = false;
  bool b3 = false;
  bool b4 = false;
  float4 x_114 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_32 = asfloat(x_10[0]);
  const float x_33 = x_32.y;
  f = x_33;
  const float4 x_37 = asfloat(x_10[0]);
  const int x_39 = int(x_37.y);
  i = x_39;
  const float4 x_44 = asfloat(x_10[0]);
  const uint x_46 = uint(x_44.y);
  u = x_46;
  const float4 x_50 = asfloat(x_10[0]);
  const bool x_52 = !((x_50.y == 0.0f));
  b = x_52;
  f1 = x_33;
  const float x_55 = float(x_39);
  f2 = x_55;
  const float x_57 = float(x_46);
  f3 = x_57;
  const float x_59 = (x_52 ? 1.0f : 0.0f);
  f4 = x_59;
  const int x_62 = int(x_33);
  i1 = x_62;
  i2 = x_39;
  const int x_65 = asint(x_46);
  i3 = x_65;
  const int x_67 = (x_52 ? 1 : 0);
  i4 = x_67;
  const uint x_70 = uint(x_33);
  u1 = x_70;
  const uint x_72 = asuint(x_39);
  u2 = x_72;
  u3 = x_46;
  const uint x_75 = (x_52 ? 1u : 0u);
  u4 = x_75;
  const bool x_79 = !((x_33 == 0.0f));
  b1 = x_79;
  const bool x_81 = (x_39 != 0);
  b2 = x_81;
  const bool x_83 = (x_46 != 0u);
  b3 = x_83;
  b4 = x_52;
  if (((((((((((((((((x_33 + x_55) + x_57) + x_59) + float(x_62)) + float(x_39)) + float(x_65)) + float(x_67)) + float(x_70)) + float(x_72)) + float(x_46)) + float(x_75)) + (x_79 ? 1.0f : 0.0f)) + (x_81 ? 1.0f : 0.0f)) + (x_83 ? 1.0f : 0.0f)) + (x_52 ? 1.0f : 0.0f)) == 16.0f)) {
    const float4 x_120 = asfloat(x_10[0]);
    x_114 = x_120;
  } else {
    const float4 x_122 = asfloat(x_10[1]);
    x_114 = x_122;
  }
  const float4 x_123 = x_114;
  return x_123;
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
