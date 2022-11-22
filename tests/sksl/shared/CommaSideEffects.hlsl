static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[4];
};

void setToColorBlack_vh4(inout float4 x_26) {
  const float4 x_32 = asfloat(x_11[3]);
  x_26 = x_32;
  return;
}

float4 main(inout float2 x_34) {
  float4 a = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 d = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_46 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_83 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_67 = false;
  bool x_68 = false;
  bool x_74 = false;
  bool x_75 = false;
  bool x_81 = false;
  bool x_82 = false;
  const float4 x_42 = asfloat(x_11[0]);
  b = x_42;
  const float4 x_45 = asfloat(x_11[1]);
  c = x_45;
  setToColorBlack_vh4(x_46);
  const float4 x_48 = x_46;
  d = x_48;
  const float4 x_51 = asfloat(x_11[2]);
  a = x_51;
  const float4 x_52 = (x_51 * x_51);
  a = x_52;
  const float4 x_53 = (x_42 * x_42);
  b = x_53;
  const float4 x_54 = (x_45 * x_45);
  c = x_54;
  const float4 x_55 = (x_48 * x_48);
  d = x_55;
  const float4 x_58 = asfloat(x_11[2]);
  x_68 = false;
  if (all((x_52 == x_58))) {
    const float4 x_65 = asfloat(x_11[0]);
    x_67 = all((x_53 == x_65));
    x_68 = x_67;
  }
  x_75 = false;
  if (x_68) {
    const float4 x_72 = asfloat(x_11[1]);
    x_74 = all((x_54 == x_72));
    x_75 = x_74;
  }
  x_82 = false;
  if (x_75) {
    const float4 x_79 = asfloat(x_11[3]);
    x_81 = all((x_55 == x_79));
    x_82 = x_81;
  }
  if (x_82) {
    const float4 x_88 = asfloat(x_11[1]);
    x_83 = x_88;
  } else {
    const float4 x_90 = asfloat(x_11[0]);
    x_83 = x_90;
  }
  const float4 x_91 = x_83;
  return x_91;
}

void _entrypoint_1() {
  float2 x_21 = float2(0.0f, 0.0f);
  x_21 = (0.0f).xx;
  const float4 x_23 = main(x_21);
  sk_FragColor = x_23;
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
