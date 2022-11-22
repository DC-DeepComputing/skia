static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  bool4 inputVal = bool4(false, false, false, false);
  bool4 expected = bool4(false, false, false, false);
  float4 x_99 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_70 = false;
  bool x_71 = false;
  bool x_76 = false;
  bool x_77 = false;
  bool x_84 = false;
  bool x_85 = false;
  bool x_91 = false;
  bool x_92 = false;
  bool x_97 = false;
  bool x_98 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const bool4 x_42 = bool4(!((x_33.x == 0.0f)), !((x_33.y == 0.0f)), !((x_33.z == 0.0f)), !((x_33.w == 0.0f)));
  inputVal = x_42;
  const float4 x_46 = asfloat(x_10[1]);
  const bool4 x_55 = bool4(!((x_46.x == 0.0f)), !((x_46.y == 0.0f)), !((x_46.z == 0.0f)), !((x_46.w == 0.0f)));
  expected = x_55;
  x_71 = false;
  if (all((!(bool2(x_42.x, x_42.y)) == bool2(x_55.x, x_55.y)))) {
    x_70 = all((!(bool3(x_42.x, x_42.y, x_42.z)) == bool3(x_55.x, x_55.y, x_55.z)));
    x_71 = x_70;
  }
  x_77 = false;
  if (x_71) {
    x_76 = all((!(x_42) == x_55));
    x_77 = x_76;
  }
  x_85 = false;
  if (x_77) {
    x_84 = all((bool2(false, true) == bool2(x_55.x, x_55.y)));
    x_85 = x_84;
  }
  x_92 = false;
  if (x_85) {
    x_91 = all((bool3(false, true, false) == bool3(x_55.x, x_55.y, x_55.z)));
    x_92 = x_91;
  }
  x_98 = false;
  if (x_92) {
    x_97 = all((bool4(false, true, false, true) == x_55));
    x_98 = x_97;
  }
  if (x_98) {
    const float4 x_106 = asfloat(x_10[2]);
    x_99 = x_106;
  } else {
    const float4 x_109 = asfloat(x_10[3]);
    x_99 = x_109;
  }
  const float4 x_110 = x_99;
  return x_110;
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
