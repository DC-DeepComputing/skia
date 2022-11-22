static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

void main_1() {
  bool4 expectTTFF = bool4(false, false, false, false);
  bool4 expectFFTT = bool4(false, false, false, false);
  bool4 expectTTTT = bool4(false, false, false, false);
  bool x_73 = false;
  bool x_75 = false;
  bool x_78 = false;
  bool x_80 = false;
  expectTTFF = bool4(true, true, false, false);
  expectFFTT = bool4(false, false, true, true);
  expectTTTT = (true).xxxx;
  const float4 x_34 = asfloat(x_10[0]);
  const float4 x_37 = asfloat(x_10[1]);
  sk_FragColor.x = float(((x_34 == x_37).x ? 1 : 0));
  const uint2 x_47 = x_10[2].xy;
  const uint2 x_50 = x_10[2].zw;
  sk_FragColor.y = float(((x_47 == x_50).y ? 1 : 0));
  const int3 x_60 = asint(x_10[3].xyz);
  const int3 x_63 = asint(x_10[4].xyz);
  sk_FragColor.z = float(((x_60 == x_63).z ? 1 : 0));
  const bool4 x_70 = expectTTFF;
  x_75 = true;
  if (any(x_70)) {
  } else {
    const bool4 x_74 = expectFFTT;
    x_73 = any(x_74);
    x_75 = x_73;
  }
  x_80 = true;
  if (x_75) {
  } else {
    const bool4 x_79 = expectTTTT;
    x_78 = any(x_79);
    x_80 = x_78;
  }
  sk_FragColor.w = float((x_80 ? 1 : 0));
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  main_1();
  const main_out tint_symbol_6 = {sk_FragColor};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
