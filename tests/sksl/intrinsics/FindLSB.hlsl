int tint_first_trailing_bit(int v) {
  uint x = uint(v);
  const uint b16 = (bool((x & 65535u)) ? 0u : 16u);
  x = (x >> b16);
  const uint b8 = (bool((x & 255u)) ? 0u : 8u);
  x = (x >> b8);
  const uint b4 = (bool((x & 15u)) ? 0u : 4u);
  x = (x >> b4);
  const uint b2 = (bool((x & 3u)) ? 0u : 2u);
  x = (x >> b2);
  const uint b1 = (bool((x & 1u)) ? 0u : 1u);
  const uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return int((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

uint tint_first_trailing_bit_1(uint v) {
  uint x = uint(v);
  const uint b16 = (bool((x & 65535u)) ? 0u : 16u);
  x = (x >> b16);
  const uint b8 = (bool((x & 255u)) ? 0u : 8u);
  x = (x >> b8);
  const uint b4 = (bool((x & 15u)) ? 0u : 4u);
  x = (x >> b4);
  const uint b2 = (bool((x & 3u)) ? 0u : 2u);
  x = (x >> b2);
  const uint b1 = (bool((x & 1u)) ? 0u : 1u);
  const uint is_zero = ((x == 0u) ? 4294967295u : 0u);
  return uint((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

void main_1() {
  const int x_22 = asint(x_10[0].x);
  const int tint_symbol_3 = tint_first_trailing_bit(x_22);
  const float tint_symbol_4 = float(tint_symbol_3);
  sk_FragColor.x = tint_symbol_4;
  const uint x_30 = x_10[0].y;
  const int tint_symbol_5 = asint(tint_first_trailing_bit_1(x_30));
  const float tint_symbol_6 = float(tint_symbol_5);
  sk_FragColor.y = tint_symbol_6;
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
  const main_out tint_symbol_9 = {sk_FragColor};
  return tint_symbol_9;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
