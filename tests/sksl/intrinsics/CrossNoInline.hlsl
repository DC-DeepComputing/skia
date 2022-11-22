static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float cross_length_2d_ff2f2(inout float2 x_18, inout float2 x_19) {
  const float2 x_22 = x_18;
  const float2 x_23 = x_19;
  return determinant(float2x2(x_22, x_23));
}

float cross_length_2d_hh2h2(inout float2 x_26, inout float2 x_27) {
  const float2 x_30 = x_26;
  const float2 x_31 = x_27;
  return determinant(float2x2(x_30, x_31));
}

void main_1() {
  float2 x_41 = float2(0.0f, 0.0f);
  float2 x_45 = float2(0.0f, 0.0f);
  float2 x_52 = float2(0.0f, 0.0f);
  float2 x_56 = float2(0.0f, 0.0f);
  const float2 x_40 = asfloat(x_12[0].xy);
  x_41 = x_40;
  const float2 x_44 = asfloat(x_12[0].zw);
  x_45 = x_44;
  const float x_46 = cross_length_2d_hh2h2(x_41, x_45);
  sk_FragColor.x = x_46;
  const float2 x_51 = asfloat(x_12[1].xy);
  x_52 = x_51;
  const float2 x_55 = asfloat(x_12[1].zw);
  x_56 = x_55;
  const float x_57 = cross_length_2d_ff2f2(x_52, x_56);
  sk_FragColor.y = x_57;
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
