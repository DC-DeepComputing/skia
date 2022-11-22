static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  bool4 v = bool4(false, false, false, false);
  bool4 result = bool4(false, false, false, false);
  float4 x_107 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_33 = asfloat(x_10[0]);
  const bool x_35 = !((x_33.y == 0.0f));
  const bool4 x_36 = bool4(x_35, x_35, x_35, x_35);
  v = x_36;
  const bool x_38 = x_36.x;
  result = bool4(x_38, true, true, true);
  const bool2 x_41 = bool2(x_36.x, x_36.y);
  result = bool4(x_41.x, x_41.y, false, true);
  result = bool4(x_38, true, true, false);
  const bool x_48 = x_36.y;
  result = bool4(false, x_48, true, true);
  const bool3 x_50 = bool3(x_36.x, x_36.y, x_36.z);
  result = bool4(x_50.x, x_50.y, x_50.z, true);
  const bool2 x_56 = bool2(x_36.x, x_36.y);
  result = bool4(x_56.x, x_56.y, true, true);
  const bool x_60 = x_36.z;
  const bool4 x_61 = bool4(x_38, false, x_60, true);
  result = x_61;
  result = bool4(x_38, true, false, false);
  const bool2 x_63 = bool2(x_36.y, x_36.z);
  result = bool4(true, x_63.x, x_63.y, false);
  result = bool4(false, x_48, true, false);
  result = bool4(true, true, x_60, false);
  result = x_36;
  const bool3 x_69 = bool3(x_36.x, x_36.y, x_36.z);
  result = bool4(x_69.x, x_69.y, x_69.z, true);
  const bool2 x_74 = bool2(x_36.x, x_36.y);
  const bool x_77 = x_36.w;
  result = bool4(x_74.x, x_74.y, false, x_77);
  const bool2 x_79 = bool2(x_36.x, x_36.y);
  result = bool4(x_79.x, x_79.y, true, false);
  const bool2 x_83 = bool2(x_36.z, x_36.w);
  result = bool4(x_38, true, x_83.x, x_83.y);
  result = x_61;
  result = bool4(x_38, true, true, x_77);
  result = bool4(x_38, true, false, true);
  const bool3 x_89 = bool3(x_36.y, x_36.z, x_36.w);
  result = bool4(true, x_89.x, x_89.y, x_89.z);
  const bool2 x_94 = bool2(x_36.y, x_36.z);
  result = bool4(false, x_94.x, x_94.y, true);
  result = bool4(false, x_48, true, x_77);
  result = bool4(true, x_48, true, true);
  const bool2 x_100 = bool2(x_36.z, x_36.w);
  result = bool4(false, false, x_100.x, x_100.y);
  result = bool4(false, false, x_60, true);
  const bool4 x_105 = bool4(false, true, true, x_77);
  result = x_105;
  if (any(x_105)) {
    const float4 x_113 = asfloat(x_10[0]);
    x_107 = x_113;
  } else {
    const float4 x_116 = asfloat(x_10[1]);
    x_107 = x_116;
  }
  const float4 x_117 = x_107;
  return x_117;
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
