static float x_kGuardedDivideEpsilon = 0.0f;
static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_17 : register(b0, space0) {
  uint4 x_17[2];
};

float color_dodge_component_Qhh2h2(inout float2 x_23, inout float2 x_24) {
  float delta = 0.0f;
  const float2 x_26 = x_24;
  if ((x_26.x == 0.0f)) {
    const float2 x_32 = x_23;
    const float2 x_35 = x_24;
    return (x_32.x * (1.0f - x_35.y));
  } else {
    const float2 x_41 = x_23;
    const float2 x_43 = x_23;
    const float x_45 = (x_41.y - x_43.x);
    delta = x_45;
    if ((x_45 == 0.0f)) {
      const float2 x_50 = x_23;
      const float2 x_52 = x_24;
      const float2 x_55 = x_23;
      const float2 x_57 = x_24;
      const float2 x_62 = x_24;
      const float2 x_64 = x_23;
      return (((x_50.y * x_52.y) + (x_55.x * (1.0f - x_57.y))) + (x_62.x * (1.0f - x_64.y)));
    } else {
      const float2 x_70 = x_24;
      const float2 x_72 = x_24;
      const float2 x_74 = x_23;
      const float x_77 = x_kGuardedDivideEpsilon;
      const float x_69 = min(x_70.y, ((x_72.x * x_74.y) / (x_45 + x_77)));
      delta = x_69;
      const float2 x_80 = x_23;
      const float2 x_83 = x_23;
      const float2 x_85 = x_24;
      const float2 x_90 = x_24;
      const float2 x_92 = x_23;
      return (((x_69 * x_80.y) + (x_83.x * (1.0f - x_85.y))) + (x_90.x * (1.0f - x_92.y)));
    }
  }
}

void main_1() {
  float2 x_106 = float2(0.0f, 0.0f);
  float2 x_111 = float2(0.0f, 0.0f);
  float2 x_116 = float2(0.0f, 0.0f);
  float2 x_120 = float2(0.0f, 0.0f);
  float2 x_125 = float2(0.0f, 0.0f);
  float2 x_129 = float2(0.0f, 0.0f);
  x_kGuardedDivideEpsilon = 0.0f;
  const float4 x_104 = asfloat(x_17[0]);
  x_106 = float2(x_104.x, x_104.w);
  const float4 x_109 = asfloat(x_17[1]);
  x_111 = float2(x_109.x, x_109.w);
  const float x_112 = color_dodge_component_Qhh2h2(x_106, x_111);
  const float4 x_114 = asfloat(x_17[0]);
  x_116 = float2(x_114.y, x_114.w);
  const float4 x_118 = asfloat(x_17[1]);
  x_120 = float2(x_118.y, x_118.w);
  const float x_121 = color_dodge_component_Qhh2h2(x_116, x_120);
  const float4 x_123 = asfloat(x_17[0]);
  x_125 = float2(x_123.z, x_123.w);
  const float4 x_127 = asfloat(x_17[1]);
  x_129 = float2(x_127.z, x_127.w);
  const float x_130 = color_dodge_component_Qhh2h2(x_125, x_129);
  const float4 x_132 = asfloat(x_17[0]);
  const float4 x_135 = asfloat(x_17[0]);
  const float4 x_139 = asfloat(x_17[1]);
  sk_FragColor = float4(x_112, x_121, x_130, (x_132.w + ((1.0f - x_135.w) * x_139.w)));
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
