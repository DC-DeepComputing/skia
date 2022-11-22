cbuffer cbuffer_x_3 : register(b1, space0) {
  uint4 x_3[1];
};
cbuffer cbuffer_x_8 : register(b2, space0) {
  uint4 x_8[1];
};
static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float2 x_24 = asfloat(x_3[0].xy);
  const float2 x_28 = asfloat(x_8[0].xy);
  sk_FragColor = float4(x_24.x, x_24.y, x_28.x, x_28.y);
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
  const main_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
