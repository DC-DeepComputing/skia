static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 sk_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_15 = sk_FragCoord;
  const float2 x_16 = float2(x_15.x, x_15.y);
  const float4 x_18 = sk_FragColor;
  sk_FragColor = float4(x_16.x, x_16.y, x_18.z, x_18.w);
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  float4 sk_FragCoord_param : SV_Position;
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param, float4 sk_FragCoord_param) {
  sk_Clockwise = sk_Clockwise_param;
  sk_FragCoord = sk_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param, tint_symbol.sk_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
