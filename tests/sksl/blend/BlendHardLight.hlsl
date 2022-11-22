static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float blend_overlay_component_Qhh2h2(inout float2 x_18, inout float2 x_19) {
  float x_28 = 0.0f;
  const float2 x_22 = x_19;
  const float2 x_25 = x_19;
  if (((2.0f * x_22.x) <= x_25.y)) {
    const float2 x_33 = x_18;
    const float2 x_36 = x_19;
    x_28 = ((2.0f * x_33.x) * x_36.x);
  } else {
    const float2 x_39 = x_18;
    const float2 x_41 = x_19;
    const float2 x_44 = x_19;
    const float2 x_46 = x_19;
    const float2 x_50 = x_18;
    const float2 x_52 = x_18;
    x_28 = ((x_39.y * x_41.y) - ((2.0f * (x_44.y - x_46.x)) * (x_50.y - x_52.x)));
  }
  const float x_57 = x_28;
  return x_57;
}

float4 blend_overlay_h4h4h4(inout float4 x_60, inout float4 x_61) {
  float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 x_66 = float2(0.0f, 0.0f);
  float2 x_69 = float2(0.0f, 0.0f);
  float2 x_73 = float2(0.0f, 0.0f);
  float2 x_76 = float2(0.0f, 0.0f);
  float2 x_80 = float2(0.0f, 0.0f);
  float2 x_83 = float2(0.0f, 0.0f);
  const float4 x_64 = x_60;
  x_66 = float2(x_64.x, x_64.w);
  const float4 x_67 = x_61;
  x_69 = float2(x_67.x, x_67.w);
  const float x_70 = blend_overlay_component_Qhh2h2(x_66, x_69);
  const float4 x_71 = x_60;
  x_73 = float2(x_71.y, x_71.w);
  const float4 x_74 = x_61;
  x_76 = float2(x_74.y, x_74.w);
  const float x_77 = blend_overlay_component_Qhh2h2(x_73, x_76);
  const float4 x_78 = x_60;
  x_80 = float2(x_78.z, x_78.w);
  const float4 x_81 = x_61;
  x_83 = float2(x_81.z, x_81.w);
  const float x_84 = blend_overlay_component_Qhh2h2(x_80, x_83);
  const float4 x_85 = x_60;
  const float4 x_88 = x_60;
  const float4 x_91 = x_61;
  result = float4(x_70, x_77, x_84, (x_85.w + ((1.0f - x_88.w) * x_91.w)));
  const float4 x_96 = result;
  const float4 x_99 = x_61;
  const float4 x_101 = x_60;
  const float4 x_105 = x_60;
  const float4 x_107 = x_61;
  const float3 x_112 = (float3(x_96.x, x_96.y, x_96.z) + ((float3(x_99.x, x_99.y, x_99.z) * (1.0f - x_101.w)) + (float3(x_105.x, x_105.y, x_105.z) * (1.0f - x_107.w))));
  const float4 x_113 = result;
  const float4 x_114 = float4(x_112.x, x_112.y, x_112.z, x_113.w);
  result = x_114;
  return x_114;
}

void main_1() {
  float4 x_123 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_127 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_122 = asfloat(x_12[1]);
  x_123 = x_122;
  const float4 x_126 = asfloat(x_12[0]);
  x_127 = x_126;
  const float4 x_128 = blend_overlay_h4h4h4(x_123, x_127);
  sk_FragColor = x_128;
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
