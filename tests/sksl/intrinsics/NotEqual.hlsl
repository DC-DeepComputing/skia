static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

void main_1() {
  bool4 expectFFTT = bool4(false, false, false, false);
  bool4 expectTTFF = bool4(false, false, false, false);
  bool x_71 = false;
  bool x_73 = false;
  expectFFTT = bool4(false, false, true, true);
  expectTTFF = bool4(true, true, false, false);
  const float4 x_32 = asfloat(x_10[0]);
  const float4 x_35 = asfloat(x_10[1]);
  sk_FragColor.x = float((!((x_32 == x_35)).x ? 1 : 0));
  const uint2 x_45 = x_10[2].xy;
  const uint2 x_48 = x_10[2].zw;
  sk_FragColor.y = float(((x_45 != x_48).y ? 1 : 0));
  const int3 x_58 = asint(x_10[3].xyz);
  const int3 x_61 = asint(x_10[4].xyz);
  sk_FragColor.z = float(((x_58 != x_61).z ? 1 : 0));
  const bool4 x_68 = expectTTFF;
  x_73 = true;
  if (any(x_68)) {
  } else {
    const bool4 x_72 = expectFFTT;
    x_71 = any(x_72);
    x_73 = x_71;
  }
  sk_FragColor.w = float((x_73 ? 1 : 0));
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
