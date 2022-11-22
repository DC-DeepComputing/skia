static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 expectedA = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedB = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_167 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_53 = false;
  bool x_54 = false;
  bool x_66 = false;
  bool x_67 = false;
  bool x_76 = false;
  bool x_77 = false;
  bool x_81 = false;
  bool x_87 = false;
  bool x_88 = false;
  bool x_94 = false;
  bool x_95 = false;
  bool x_98 = false;
  bool x_109 = false;
  bool x_110 = false;
  bool x_122 = false;
  bool x_123 = false;
  bool x_135 = false;
  bool x_136 = false;
  bool x_145 = false;
  bool x_146 = false;
  bool x_149 = false;
  bool x_155 = false;
  bool x_156 = false;
  bool x_162 = false;
  bool x_163 = false;
  bool x_166 = false;
  expectedA = float4(0.75f, 0.0f, 0.75f, 0.25f);
  expectedB = float4(0.25f, 0.0f, 0.75f, 1.0f);
  const float4 x_40 = asfloat(x_10[0]);
  x_54 = false;
  if (((x_40.x - (1.0f * floor((x_40.x / 1.0f)))) == 0.75f)) {
    const float4 x_47 = asfloat(x_10[0]);
    x_53 = all(((float2(x_47.x, x_47.y) - ((1.0f).xx * floor((float2(x_47.x, x_47.y) / (1.0f).xx)))) == float2(0.75f, 0.0f)));
    x_54 = x_53;
  }
  x_67 = false;
  if (x_54) {
    const float4 x_59 = asfloat(x_10[0]);
    x_66 = all(((float3(x_59.x, x_59.y, x_59.z) - ((1.0f).xxx * floor((float3(x_59.x, x_59.y, x_59.z) / (1.0f).xxx)))) == float3(0.75f, 0.0f, 0.75f)));
    x_67 = x_66;
  }
  x_77 = false;
  if (x_67) {
    const float4 x_72 = asfloat(x_10[0]);
    x_76 = all(((x_72 - ((1.0f).xxxx * floor((x_72 / (1.0f).xxxx)))) == float4(0.75f, 0.0f, 0.75f, 0.25f)));
    x_77 = x_76;
  }
  x_81 = false;
  if (x_77) {
    x_81 = true;
  }
  x_88 = false;
  if (x_81) {
    x_87 = true;
    x_88 = x_87;
  }
  x_95 = false;
  if (x_88) {
    x_94 = true;
    x_95 = x_94;
  }
  x_98 = false;
  if (x_95) {
    x_98 = true;
  }
  x_110 = false;
  if (x_98) {
    const float4 x_103 = asfloat(x_10[0]);
    const float4 x_107 = asfloat(x_10[3]);
    x_109 = ((x_103.x - (x_107.x * floor((x_103.x / x_107.x)))) == 0.75f);
    x_110 = x_109;
  }
  x_123 = false;
  if (x_110) {
    const float4 x_115 = asfloat(x_10[0]);
    const float4 x_118 = asfloat(x_10[3]);
    x_122 = all(((float2(x_115.x, x_115.y) - (float2(x_118.x, x_118.y) * floor((float2(x_115.x, x_115.y) / float2(x_118.x, x_118.y))))) == float2(0.75f, 0.0f)));
    x_123 = x_122;
  }
  x_136 = false;
  if (x_123) {
    const float4 x_128 = asfloat(x_10[0]);
    const float4 x_131 = asfloat(x_10[3]);
    x_135 = all(((float3(x_128.x, x_128.y, x_128.z) - (float3(x_131.x, x_131.y, x_131.z) * floor((float3(x_128.x, x_128.y, x_128.z) / float3(x_131.x, x_131.y, x_131.z))))) == float3(0.75f, 0.0f, 0.75f)));
    x_136 = x_135;
  }
  x_146 = false;
  if (x_136) {
    const float4 x_141 = asfloat(x_10[0]);
    const float4 x_143 = asfloat(x_10[3]);
    x_145 = all(((x_141 - (x_143 * floor((x_141 / x_143)))) == float4(0.75f, 0.0f, 0.75f, 0.25f)));
    x_146 = x_145;
  }
  x_149 = false;
  if (x_146) {
    x_149 = true;
  }
  x_156 = false;
  if (x_149) {
    x_155 = true;
    x_156 = x_155;
  }
  x_163 = false;
  if (x_156) {
    x_162 = true;
    x_163 = x_162;
  }
  x_166 = false;
  if (x_163) {
    x_166 = true;
  }
  if (x_166) {
    const float4 x_173 = asfloat(x_10[1]);
    x_167 = x_173;
  } else {
    const float4 x_176 = asfloat(x_10[2]);
    x_167 = x_176;
  }
  const float4 x_177 = x_167;
  return x_177;
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
