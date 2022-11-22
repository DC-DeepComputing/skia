static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1025];
};

void main_1() {
  bool x_25 = false;
  const float2 x_21 = asfloat(x_13[1024].xy);
  if ((x_21.y > 0.0f)) {
    const bool x_31 = sk_Clockwise;
    x_25 = !(x_31);
  } else {
    const bool x_32 = sk_Clockwise;
    x_25 = x_32;
  }
  const bool x_33 = x_25;
  const float x_37 = float((x_33 ? 1 : -1));
  sk_FragColor = float4(x_37, x_37, x_37, x_37);
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
  const main_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
