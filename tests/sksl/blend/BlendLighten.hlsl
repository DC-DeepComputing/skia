static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  float4 x_0_result = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_22 = asfloat(x_10[0]);
  const float4 x_25 = asfloat(x_10[0]);
  const float4 x_30 = asfloat(x_10[1]);
  const float4 x_32 = (x_22 + (x_30 * (1.0f - x_25.w)));
  x_0_result = x_32;
  const float4 x_37 = asfloat(x_10[1]);
  const float4 x_41 = asfloat(x_10[0]);
  const float4 x_45 = asfloat(x_10[1]);
  const float3 x_33 = max(float3(x_32.x, x_32.y, x_32.z), ((float3(x_41.x, x_41.y, x_41.z) * (1.0f - x_37.w)) + float3(x_45.x, x_45.y, x_45.z)));
  const float4 x_48 = x_0_result;
  const float4 x_49 = float4(x_33.x, x_33.y, x_33.z, x_48.w);
  x_0_result = x_49;
  sk_FragColor = x_49;
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
