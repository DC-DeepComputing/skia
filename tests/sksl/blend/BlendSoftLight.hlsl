static float x_kGuardedDivideEpsilon = 0.0f;
static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_17 : register(b0, space0) {
  uint4 x_17[2];
};

float soft_light_component_Qhh2h2(inout float2 x_23, inout float2 x_24) {
  float DSqd = 0.0f;
  float DCub = 0.0f;
  float DaSqd = 0.0f;
  float DaCub = 0.0f;
  const float2 x_27 = x_23;
  const float2 x_30 = x_23;
  if (((2.0f * x_27.x) <= x_30.y)) {
    const float2 x_36 = x_24;
    const float2 x_38 = x_24;
    const float2 x_41 = x_23;
    const float2 x_43 = x_23;
    const float2 x_48 = x_24;
    const float x_50 = x_kGuardedDivideEpsilon;
    const float2 x_54 = x_24;
    const float2 x_57 = x_23;
    const float2 x_61 = x_24;
    const float2 x_63 = x_23;
    const float2 x_66 = x_23;
    return (((((x_36.x * x_38.x) * (x_41.y - (2.0f * x_43.x))) / (x_48.y + x_50)) + ((1.0f - x_54.y) * x_57.x)) + (x_61.x * ((-(x_63.y) + (2.0f * x_66.x)) + 1.0f)));
  } else {
    const float2 x_74 = x_24;
    const float2 x_77 = x_24;
    if (((4.0f * x_74.x) <= x_77.y)) {
      const float2 x_85 = x_24;
      const float2 x_87 = x_24;
      const float x_89 = (x_85.x * x_87.x);
      DSqd = x_89;
      const float2 x_91 = x_24;
      const float x_93 = (x_89 * x_91.x);
      DCub = x_93;
      const float2 x_95 = x_24;
      const float2 x_97 = x_24;
      const float x_99 = (x_95.y * x_97.y);
      DaSqd = x_99;
      const float2 x_101 = x_24;
      const float x_103 = (x_99 * x_101.y);
      DaCub = x_103;
      const float2 x_104 = x_23;
      const float2 x_106 = x_24;
      const float2 x_109 = x_23;
      const float2 x_113 = x_23;
      const float2 x_122 = x_24;
      const float2 x_126 = x_23;
      const float2 x_128 = x_23;
      const float2 x_136 = x_23;
      const float2 x_138 = x_23;
      const float2 x_144 = x_23;
      const float x_148 = x_kGuardedDivideEpsilon;
      return (((((x_99 * (x_104.x - (x_106.x * (((3.0f * x_109.y) - (6.0f * x_113.x)) - 1.0f)))) + (((12.0f * x_122.y) * x_89) * (x_126.y - (2.0f * x_128.x)))) - ((16.0f * x_93) * (x_136.y - (2.0f * x_138.x)))) - (x_103 * x_144.x)) / (x_99 + x_148));
    } else {
      const float2 x_151 = x_24;
      const float2 x_153 = x_23;
      const float2 x_155 = x_23;
      const float2 x_161 = x_23;
      const float2 x_165 = x_24;
      const float2 x_167 = x_24;
      const float2 x_170 = x_23;
      const float2 x_172 = x_23;
      const float2 x_178 = x_24;
      const float2 x_180 = x_23;
      return ((((x_151.x * ((x_153.y - (2.0f * x_155.x)) + 1.0f)) + x_161.x) - (sqrt((x_165.y * x_167.x)) * (x_170.y - (2.0f * x_172.x)))) - (x_178.y * x_180.x));
    }
  }
}

void main_1() {
  float4 x_194 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 x_205 = float2(0.0f, 0.0f);
  float2 x_209 = float2(0.0f, 0.0f);
  float2 x_214 = float2(0.0f, 0.0f);
  float2 x_218 = float2(0.0f, 0.0f);
  float2 x_223 = float2(0.0f, 0.0f);
  float2 x_227 = float2(0.0f, 0.0f);
  x_kGuardedDivideEpsilon = 0.0f;
  const float4 x_191 = asfloat(x_17[1]);
  if ((x_191.w == 0.0f)) {
    const float4 x_201 = asfloat(x_17[0]);
    x_194 = x_201;
  } else {
    const float4 x_203 = asfloat(x_17[0]);
    x_205 = float2(x_203.x, x_203.w);
    const float4 x_207 = asfloat(x_17[1]);
    x_209 = float2(x_207.x, x_207.w);
    const float x_210 = soft_light_component_Qhh2h2(x_205, x_209);
    const float4 x_212 = asfloat(x_17[0]);
    x_214 = float2(x_212.y, x_212.w);
    const float4 x_216 = asfloat(x_17[1]);
    x_218 = float2(x_216.y, x_216.w);
    const float x_219 = soft_light_component_Qhh2h2(x_214, x_218);
    const float4 x_221 = asfloat(x_17[0]);
    x_223 = float2(x_221.z, x_221.w);
    const float4 x_225 = asfloat(x_17[1]);
    x_227 = float2(x_225.z, x_225.w);
    const float x_228 = soft_light_component_Qhh2h2(x_223, x_227);
    const float4 x_230 = asfloat(x_17[0]);
    const float4 x_233 = asfloat(x_17[0]);
    const float4 x_237 = asfloat(x_17[1]);
    x_194 = float4(x_210, x_219, x_228, (x_230.w + ((1.0f - x_233.w) * x_237.w)));
  }
  const float4 x_242 = x_194;
  sk_FragColor = x_242;
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
