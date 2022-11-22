static float x_kGuardedDivideEpsilon = 0.0f;
static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_17 : register(b0, space0) {
  uint4 x_17[2];
};

float color_burn_component_Qhh2h2(inout float2 x_23, inout float2 x_24) {
  float delta = 0.0f;
  const float2 x_26 = x_24;
  const float2 x_28 = x_24;
  if ((x_26.y == x_28.x)) {
    const float2 x_34 = x_23;
    const float2 x_36 = x_24;
    const float2 x_39 = x_23;
    const float2 x_42 = x_24;
    const float2 x_47 = x_24;
    const float2 x_49 = x_23;
    return (((x_34.y * x_36.y) + (x_39.x * (1.0f - x_42.y))) + (x_47.x * (1.0f - x_49.y)));
  } else {
    const float2 x_54 = x_23;
    if ((x_54.x == 0.0f)) {
      const float2 x_60 = x_24;
      const float2 x_62 = x_23;
      return (x_60.x * (1.0f - x_62.y));
    } else {
      const float2 x_69 = x_24;
      const float2 x_71 = x_24;
      const float2 x_73 = x_24;
      const float2 x_76 = x_23;
      const float2 x_79 = x_23;
      const float x_81 = x_kGuardedDivideEpsilon;
      const float x_68 = max(0.0f, (x_69.y - (((x_71.y - x_73.x) * x_76.y) / (x_79.x + x_81))));
      delta = x_68;
      const float2 x_85 = x_23;
      const float2 x_88 = x_23;
      const float2 x_90 = x_24;
      const float2 x_95 = x_24;
      const float2 x_97 = x_23;
      return (((x_68 * x_85.y) + (x_88.x * (1.0f - x_90.y))) + (x_95.x * (1.0f - x_97.y)));
    }
  }
}

void main_1() {
  float2 x_111 = float2(0.0f, 0.0f);
  float2 x_116 = float2(0.0f, 0.0f);
  float2 x_121 = float2(0.0f, 0.0f);
  float2 x_125 = float2(0.0f, 0.0f);
  float2 x_130 = float2(0.0f, 0.0f);
  float2 x_134 = float2(0.0f, 0.0f);
  x_kGuardedDivideEpsilon = 0.0f;
  const float4 x_109 = asfloat(x_17[0]);
  x_111 = float2(x_109.x, x_109.w);
  const float4 x_114 = asfloat(x_17[1]);
  x_116 = float2(x_114.x, x_114.w);
  const float x_117 = color_burn_component_Qhh2h2(x_111, x_116);
  const float4 x_119 = asfloat(x_17[0]);
  x_121 = float2(x_119.y, x_119.w);
  const float4 x_123 = asfloat(x_17[1]);
  x_125 = float2(x_123.y, x_123.w);
  const float x_126 = color_burn_component_Qhh2h2(x_121, x_125);
  const float4 x_128 = asfloat(x_17[0]);
  x_130 = float2(x_128.z, x_128.w);
  const float4 x_132 = asfloat(x_17[1]);
  x_134 = float2(x_132.z, x_132.w);
  const float x_135 = color_burn_component_Qhh2h2(x_130, x_134);
  const float4 x_137 = asfloat(x_17[0]);
  const float4 x_140 = asfloat(x_17[0]);
  const float4 x_144 = asfloat(x_17[1]);
  sk_FragColor = float4(x_117, x_126, x_135, (x_137.w + ((1.0f - x_140.w) * x_144.w)));
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
