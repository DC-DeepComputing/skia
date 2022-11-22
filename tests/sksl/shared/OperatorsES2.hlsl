static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float x = 0.0f;
  float y = 0.0f;
  int z = 0;
  bool b = false;
  bool c = false;
  bool d = false;
  bool e = false;
  bool f = false;
  float4 x_107 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_65 = false;
  bool x_66 = false;
  bool x_67 = false;
  bool x_77 = false;
  bool x_81 = false;
  bool x_103 = false;
  bool x_106 = false;
  x = 1.0f;
  y = 2.0f;
  z = 3;
  const float x_40 = 2.0f;
  x = x_40;
  const float x_42 = ((x_40 / 2.0f) / x_40);
  y = x_42;
  const int x_48 = 5;
  z = x_48;
  x_67 = true;
  if (((x_40 > 4.0f) == (x_40 < 2.0f))) {
  } else {
    const float x_61 = asfloat(x_10[2].x);
    x_66 = false;
    if ((2.0f >= x_61)) {
      x_65 = (x_42 <= x_40);
      x_66 = x_65;
    }
    x_67 = x_66;
  }
  b = x_67;
  const float x_70 = asfloat(x_10[2].x);
  const bool x_71 = (x_70 > 2.0f);
  c = x_71;
  const bool x_73 = (x_67 != x_71);
  d = x_73;
  x_77 = false;
  if (x_67) {
    x_77 = x_71;
  }
  e = x_77;
  x_81 = true;
  if (x_67) {
  } else {
    x_81 = x_71;
  }
  f = x_81;
  const float x_83 = (x_40 + 12.0f);
  x = x_83;
  const float x_84 = (x_83 - 12.0f);
  x = x_84;
  const float x_86 = (x_42 / 10.0f);
  y = x_86;
  x = (x_84 * x_86);
  x = 6.0f;
  y = (((((x_67 ? 1.0f : 0.0f) * (x_71 ? 1.0f : 0.0f)) * (x_73 ? 1.0f : 0.0f)) * (x_77 ? 1.0f : 0.0f)) * (x_81 ? 1.0f : 0.0f));
  y = 6.0f;
  z = (x_48 - 1);
  z = 6;
  x_103 = false;
  if (true) {
    x_103 = true;
  }
  x_106 = false;
  if (x_103) {
    x_106 = true;
  }
  if (x_106) {
    const float4 x_115 = asfloat(x_10[0]);
    x_107 = x_115;
  } else {
    const float4 x_117 = asfloat(x_10[1]);
    x_107 = x_117;
  }
  const float4 x_118 = x_107;
  return x_118;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
