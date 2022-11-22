static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 sk_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_15 : register(b0, space0) {
  uint4 x_15[1025];
};

void main_1() {
  const float4 x_19 = sk_FragCoord;
  const float2 x_25 = asfloat(x_15[1024].xy);
  const float2 x_28 = asfloat(x_15[1024].xy);
  const float4 x_30 = sk_FragCoord;
  const float4 x_34 = sk_FragCoord;
  const float4 x_36 = sk_FragCoord;
  const float4 x_38 = float4(x_19.x, (x_25.x + (x_28.y * x_30.y)), x_34.z, x_36.w);
  const float2 x_39 = float2(x_38.x, x_38.y);
  const float4 x_40 = sk_FragColor;
  sk_FragColor = float4(x_39.x, x_39.y, x_40.z, x_40.w);
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
  const main_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param, tint_symbol.sk_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
