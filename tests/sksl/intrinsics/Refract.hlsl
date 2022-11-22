static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

void main_1() {
  const float x_21 = asfloat(x_10[0].x);
  const float x_24 = asfloat(x_10[0].y);
  const float x_27 = asfloat(x_10[0].z);
  sk_FragColor.x = refract(float2(x_21, 0.0f), float2(x_24, 0.0f), x_27).x;
  const float4 x_34 = asfloat(x_10[1]);
  const float4 x_37 = asfloat(x_10[2]);
  const float x_39 = asfloat(x_10[0].z);
  sk_FragColor = refract(x_34, x_37, x_39);
  const float4 x_44 = sk_FragColor;
  sk_FragColor = float4(0.5f, -0.866025388f, x_44.z, x_44.w);
  const float4 x_49 = sk_FragColor;
  sk_FragColor = float4(0.5f, 0.0f, -0.866025388f, x_49.w);
  sk_FragColor = float4(0.5f, 0.0f, 0.0f, -0.866025388f);
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
