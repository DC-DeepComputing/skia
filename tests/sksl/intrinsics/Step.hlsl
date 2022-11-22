static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expectedA = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedB = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_156 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_52 = false;
  bool x_53 = false;
  bool x_65 = false;
  bool x_66 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_80 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_92 = false;
  bool x_93 = false;
  bool x_96 = false;
  bool x_103 = false;
  bool x_104 = false;
  bool x_114 = false;
  bool x_115 = false;
  bool x_125 = false;
  bool x_126 = false;
  bool x_134 = false;
  bool x_135 = false;
  bool x_138 = false;
  bool x_144 = false;
  bool x_145 = false;
  bool x_151 = false;
  bool x_152 = false;
  bool x_155 = false;
  expectedA = float4(0.0f, 0.0f, 1.0f, 1.0f);
  expectedB = float4(1.0f, 1.0f, 0.0f, 0.0f);
  const float4 x_39 = asfloat(x_10[0]);
  x_53 = false;
  if ((step(0.5f, x_39.x) == 0.0f)) {
    const float4 x_47 = asfloat(x_10[0]);
    x_52 = all((step((0.5f).xx, float2(x_47.x, x_47.y)) == (0.0f).xx));
    x_53 = x_52;
  }
  x_66 = false;
  if (x_53) {
    const float4 x_60 = asfloat(x_10[0]);
    x_65 = all((step((0.5f).xxx, float3(x_60.x, x_60.y, x_60.z)) == float3(0.0f, 0.0f, 1.0f)));
    x_66 = x_65;
  }
  x_76 = false;
  if (x_66) {
    const float4 x_72 = asfloat(x_10[0]);
    x_75 = all((step((0.5f).xxxx, x_72) == float4(0.0f, 0.0f, 1.0f, 1.0f)));
    x_76 = x_75;
  }
  x_80 = false;
  if (x_76) {
    x_80 = true;
  }
  x_86 = false;
  if (x_80) {
    x_85 = true;
    x_86 = x_85;
  }
  x_93 = false;
  if (x_86) {
    x_92 = true;
    x_93 = x_92;
  }
  x_96 = false;
  if (x_93) {
    x_96 = true;
  }
  x_104 = false;
  if (x_96) {
    const float4 x_101 = asfloat(x_10[0]);
    x_103 = (step(x_101.x, 0.0f) == 1.0f);
    x_104 = x_103;
  }
  x_115 = false;
  if (x_104) {
    const float4 x_109 = asfloat(x_10[0]);
    x_114 = all((step(float2(x_109.x, x_109.y), float2(0.0f, 1.0f)) == (1.0f).xx));
    x_115 = x_114;
  }
  x_126 = false;
  if (x_115) {
    const float4 x_120 = asfloat(x_10[0]);
    x_125 = all((step(float3(x_120.x, x_120.y, x_120.z), float3(0.0f, 1.0f, 0.0f)) == float3(1.0f, 1.0f, 0.0f)));
    x_126 = x_125;
  }
  x_135 = false;
  if (x_126) {
    const float4 x_131 = asfloat(x_10[0]);
    x_134 = all((step(x_131, float4(0.0f, 1.0f, 0.0f, 1.0f)) == float4(1.0f, 1.0f, 0.0f, 0.0f)));
    x_135 = x_134;
  }
  x_138 = false;
  if (x_135) {
    x_138 = true;
  }
  x_145 = false;
  if (x_138) {
    x_144 = true;
    x_145 = x_144;
  }
  x_152 = false;
  if (x_145) {
    x_151 = true;
    x_152 = x_151;
  }
  x_155 = false;
  if (x_152) {
    x_155 = true;
  }
  if (x_155) {
    const float4 x_162 = asfloat(x_10[1]);
    x_156 = x_162;
  } else {
    const float4 x_165 = asfloat(x_10[2]);
    x_156 = x_165;
  }
  const float4 x_166 = x_156;
  return x_166;
}

void _entrypoint_1() {
  float2 x_20 = float2(0.0f, 0.0f);
  x_20 = (0.0f).xx;
  const float4 x_22 = main(x_20);
  sk_FragColor = x_22;
  return;
}

struct _entrypoint_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

_entrypoint_out _entrypoint_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  _entrypoint_1();
  const _entrypoint_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
