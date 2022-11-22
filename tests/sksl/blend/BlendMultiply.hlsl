static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  const float4 x_21 = asfloat(x_10[0]);
  const float4 x_26 = asfloat(x_10[1]);
  const float4 x_31 = asfloat(x_10[1]);
  const float4 x_35 = asfloat(x_10[0]);
  const float4 x_40 = asfloat(x_10[0]);
  const float4 x_43 = asfloat(x_10[1]);
  const float3 x_46 = (((float3(x_26.x, x_26.y, x_26.z) * (1.0f - x_21.w)) + (float3(x_35.x, x_35.y, x_35.z) * (1.0f - x_31.w))) + (float3(x_40.x, x_40.y, x_40.z) * float3(x_43.x, x_43.y, x_43.z)));
  const float4 x_51 = asfloat(x_10[0]);
  const float4 x_54 = asfloat(x_10[0]);
  const float4 x_58 = asfloat(x_10[1]);
  sk_FragColor = float4(x_46.x, x_46.y, x_46.z, (x_51.w + ((1.0f - x_54.w) * x_58.w)));
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
