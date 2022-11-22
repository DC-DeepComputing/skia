static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  const float4 x_20 = asfloat(x_10[1]);
  const float4 x_25 = asfloat(x_10[0]);
  const float4 x_30 = asfloat(x_10[1]);
  const float4 x_34 = asfloat(x_10[0]);
  const float3 x_37 = ((float3(x_20.x, x_20.y, x_20.z) + float3(x_25.x, x_25.y, x_25.z)) - ((float3(x_30.x, x_30.y, x_30.z) * 2.0f) * float3(x_34.x, x_34.y, x_34.z)));
  const float4 x_42 = asfloat(x_10[0]);
  const float4 x_46 = asfloat(x_10[0]);
  const float4 x_50 = asfloat(x_10[1]);
  sk_FragColor = float4(x_37.x, x_37.y, x_37.z, (x_42.w + ((1.0f - x_46.w) * x_50.w)));
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
