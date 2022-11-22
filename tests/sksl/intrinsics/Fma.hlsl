static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[7];
};

float4 main(inout float2 x_27) {
  float one = 0.0f;
  float two = 0.0f;
  float three = 0.0f;
  float four = 0.0f;
  float five = 0.0f;
  float4 x_67 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_65 = false;
  bool x_66 = false;
  const float x_37 = asfloat(x_10[2].x);
  one = x_37;
  const float x_42 = asfloat(x_10[3].x);
  two = x_42;
  const float x_46 = asfloat(x_10[4].x);
  three = x_46;
  const float x_51 = asfloat(x_10[5].x);
  four = x_51;
  const float x_56 = asfloat(x_10[6].x);
  five = x_56;
  x_66 = false;
  if ((mad(x_37, x_42, x_46) == 5.0f)) {
    x_65 = (mad(x_46, x_51, x_56) == 17.0f);
    x_66 = x_65;
  }
  if (x_66) {
    const float4 x_74 = asfloat(x_10[0]);
    x_67 = x_74;
  } else {
    const float4 x_76 = asfloat(x_10[1]);
    x_67 = x_76;
  }
  const float4 x_77 = x_67;
  return x_77;
}

void _entrypoint_1() {
  float2 x_23 = float2(0.0f, 0.0f);
  x_23 = (0.0f).xx;
  const float4 x_25 = main(x_23);
  sk_FragColor = x_25;
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
