static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  bool4 inputVal = bool4(false, false, false, false);
  bool4 expected = bool4(false, false, false, false);
  float4 x_88 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_68 = false;
  bool x_69 = false;
  bool x_74 = false;
  bool x_75 = false;
  bool x_78 = false;
  bool x_79 = false;
  bool x_82 = false;
  bool x_83 = false;
  bool x_86 = false;
  bool x_87 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const bool4 x_42 = bool4(!((x_33.x == 0.0f)), !((x_33.y == 0.0f)), !((x_33.z == 0.0f)), !((x_33.w == 0.0f)));
  inputVal = x_42;
  const float4 x_46 = asfloat(x_10[1]);
  const bool4 x_55 = bool4(!((x_46.x == 0.0f)), !((x_46.y == 0.0f)), !((x_46.z == 0.0f)), !((x_46.w == 0.0f)));
  expected = x_55;
  const bool x_60 = x_55.x;
  x_69 = false;
  if ((any(bool2(x_42.x, x_42.y)) == x_60)) {
    x_68 = (any(bool3(x_42.x, x_42.y, x_42.z)) == x_55.y);
    x_69 = x_68;
  }
  x_75 = false;
  if (x_69) {
    x_74 = (any(x_42) == x_55.z);
    x_75 = x_74;
  }
  x_79 = false;
  if (x_75) {
    x_78 = (false == x_60);
    x_79 = x_78;
  }
  x_83 = false;
  if (x_79) {
    x_82 = x_55.y;
    x_83 = x_82;
  }
  x_87 = false;
  if (x_83) {
    x_86 = x_55.z;
    x_87 = x_86;
  }
  if (x_87) {
    const float4 x_95 = asfloat(x_10[2]);
    x_88 = x_95;
  } else {
    const float4 x_98 = asfloat(x_10[3]);
    x_88 = x_98;
  }
  const float4 x_99 = x_88;
  return x_99;
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
