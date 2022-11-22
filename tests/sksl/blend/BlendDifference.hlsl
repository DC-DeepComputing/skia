static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  const float4 x_20 = asfloat(x_10[0]);
  const float4 x_25 = asfloat(x_10[1]);
  const float4 x_31 = asfloat(x_10[0]);
  const float4 x_34 = asfloat(x_10[1]);
  const float4 x_38 = asfloat(x_10[1]);
  const float4 x_41 = asfloat(x_10[0]);
  const float3 x_45 = ((float3(x_20.x, x_20.y, x_20.z) + float3(x_25.x, x_25.y, x_25.z)) - (min((float3(x_31.x, x_31.y, x_31.z) * x_34.w), (float3(x_38.x, x_38.y, x_38.z) * x_41.w)) * 2.0f));
  const float4 x_50 = asfloat(x_10[0]);
  const float4 x_54 = asfloat(x_10[0]);
  const float4 x_58 = asfloat(x_10[1]);
  sk_FragColor = float4(x_45.x, x_45.y, x_45.z, (x_50.w + ((1.0f - x_54.w) * x_58.w)));
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
