static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

float blend_overlay_component_Qhh2h2(inout float2 x_17, inout float2 x_18) {
  float x_27 = 0.0f;
  const float2 x_21 = x_18;
  const float2 x_24 = x_18;
  if (((2.0f * x_21.x) <= x_24.y)) {
    const float2 x_32 = x_17;
    const float2 x_35 = x_18;
    x_27 = ((2.0f * x_32.x) * x_35.x);
  } else {
    const float2 x_38 = x_17;
    const float2 x_40 = x_18;
    const float2 x_43 = x_18;
    const float2 x_45 = x_18;
    const float2 x_49 = x_17;
    const float2 x_51 = x_17;
    x_27 = ((x_38.y * x_40.y) - ((2.0f * (x_43.y - x_45.x)) * (x_49.y - x_51.x)));
  }
  const float x_56 = x_27;
  return x_56;
}

void main_1() {
  float4 x_0_result = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 x_68 = float2(0.0f, 0.0f);
  float2 x_73 = float2(0.0f, 0.0f);
  float2 x_78 = float2(0.0f, 0.0f);
  float2 x_82 = float2(0.0f, 0.0f);
  float2 x_87 = float2(0.0f, 0.0f);
  float2 x_91 = float2(0.0f, 0.0f);
  const float4 x_66 = asfloat(x_11[0]);
  x_68 = float2(x_66.x, x_66.w);
  const float4 x_71 = asfloat(x_11[1]);
  x_73 = float2(x_71.x, x_71.w);
  const float x_74 = blend_overlay_component_Qhh2h2(x_68, x_73);
  const float4 x_76 = asfloat(x_11[0]);
  x_78 = float2(x_76.y, x_76.w);
  const float4 x_80 = asfloat(x_11[1]);
  x_82 = float2(x_80.y, x_80.w);
  const float x_83 = blend_overlay_component_Qhh2h2(x_78, x_82);
  const float4 x_85 = asfloat(x_11[0]);
  x_87 = float2(x_85.z, x_85.w);
  const float4 x_89 = asfloat(x_11[1]);
  x_91 = float2(x_89.z, x_89.w);
  const float x_92 = blend_overlay_component_Qhh2h2(x_87, x_91);
  const float4 x_94 = asfloat(x_11[0]);
  const float4 x_98 = asfloat(x_11[0]);
  const float4 x_102 = asfloat(x_11[1]);
  x_0_result = float4(x_74, x_83, x_92, (x_94.w + ((1.0f - x_98.w) * x_102.w)));
  const float4 x_107 = x_0_result;
  const float4 x_111 = asfloat(x_11[1]);
  const float4 x_114 = asfloat(x_11[0]);
  const float4 x_119 = asfloat(x_11[0]);
  const float4 x_122 = asfloat(x_11[1]);
  const float3 x_127 = (float3(x_107.x, x_107.y, x_107.z) + ((float3(x_111.x, x_111.y, x_111.z) * (1.0f - x_114.w)) + (float3(x_119.x, x_119.y, x_119.z) * (1.0f - x_122.w))));
  const float4 x_128 = x_0_result;
  const float4 x_129 = float4(x_127.x, x_127.y, x_127.z, x_128.w);
  x_0_result = x_129;
  sk_FragColor = x_129;
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
