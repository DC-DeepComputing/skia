struct strided_arr {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  strided_arr array_1[4] = (strided_arr[4])0;
  int x = 0;
  uint y = 0u;
  int z = 0;
  uint w = 0u;
  const strided_arr tint_symbol_3 = {1.0f};
  const strided_arr tint_symbol_4 = {2.0f};
  const strided_arr tint_symbol_5 = {3.0f};
  const strided_arr tint_symbol_6 = {4.0f};
  const strided_arr tint_symbol_7[4] = {tint_symbol_3, tint_symbol_4, tint_symbol_5, tint_symbol_6};
  array_1 = tint_symbol_7;
  x = 0;
  y = 1u;
  z = 2;
  w = 3u;
  const float x_36 = array_1[0].el;
  const float x_38 = array_1[1u].el;
  const float x_40 = array_1[2].el;
  const float x_42 = array_1[3u].el;
  sk_FragColor = float4(x_36, x_38, x_40, x_42);
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
  const main_out tint_symbol_8 = {sk_FragColor};
  return tint_symbol_8;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
