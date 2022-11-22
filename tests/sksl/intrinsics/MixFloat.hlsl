static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

float4 main(inout float2 x_24) {
  float4 expectedBW = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedWT = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_232 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_61 = false;
  bool x_62 = false;
  bool x_73 = false;
  bool x_74 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_98 = false;
  bool x_99 = false;
  bool x_113 = false;
  bool x_114 = false;
  bool x_129 = false;
  bool x_130 = false;
  bool x_140 = false;
  bool x_141 = false;
  bool x_145 = false;
  bool x_150 = false;
  bool x_151 = false;
  bool x_156 = false;
  bool x_157 = false;
  bool x_160 = false;
  bool x_171 = false;
  bool x_172 = false;
  bool x_185 = false;
  bool x_186 = false;
  bool x_199 = false;
  bool x_200 = false;
  bool x_210 = false;
  bool x_211 = false;
  bool x_214 = false;
  bool x_220 = false;
  bool x_221 = false;
  bool x_227 = false;
  bool x_228 = false;
  bool x_231 = false;
  expectedBW = float4(0.5f, 0.5f, 0.5f, 1.0f);
  expectedWT = float4(1.0f, 0.5f, 1.0f, 2.25f);
  const float4 x_40 = asfloat(x_10[0]);
  const float4 x_43 = asfloat(x_10[1]);
  x_62 = false;
  if (all((lerp(x_40, x_43, (0.0f).xxxx) == float4(0.0f, 1.0f, 0.0f, 1.0f)))) {
    const float4 x_53 = asfloat(x_10[0]);
    const float4 x_55 = asfloat(x_10[1]);
    x_61 = all((lerp(x_53, x_55, (0.25f).xxxx) == float4(0.25f, 0.75f, 0.0f, 1.0f)));
    x_62 = x_61;
  }
  x_74 = false;
  if (x_62) {
    const float4 x_67 = asfloat(x_10[0]);
    const float4 x_69 = asfloat(x_10[1]);
    x_73 = all((lerp(x_67, x_69, (0.75f).xxxx) == float4(0.75f, 0.25f, 0.0f, 1.0f)));
    x_74 = x_73;
  }
  x_86 = false;
  if (x_74) {
    const float4 x_79 = asfloat(x_10[0]);
    const float4 x_81 = asfloat(x_10[1]);
    x_85 = all((lerp(x_79, x_81, (1.0f).xxxx) == float4(1.0f, 0.0f, 0.0f, 1.0f)));
    x_86 = x_85;
  }
  x_99 = false;
  if (x_86) {
    const float4 x_92 = asfloat(x_10[2]);
    const float4 x_96 = asfloat(x_10[3]);
    x_98 = (lerp(x_92.x, x_96.x, 0.5f) == 0.5f);
    x_99 = x_98;
  }
  x_114 = false;
  if (x_99) {
    const float4 x_104 = asfloat(x_10[2]);
    const float4 x_107 = asfloat(x_10[3]);
    x_113 = all((lerp(float2(x_104.x, x_104.y), float2(x_107.x, x_107.y), (0.5f).xx) == (0.5f).xx));
    x_114 = x_113;
  }
  x_130 = false;
  if (x_114) {
    const float4 x_119 = asfloat(x_10[2]);
    const float4 x_123 = asfloat(x_10[3]);
    x_129 = all((lerp(float3(x_119.x, x_119.y, x_119.z), float3(x_123.x, x_123.y, x_123.z), (0.5f).xxx) == (0.5f).xxx));
    x_130 = x_129;
  }
  x_141 = false;
  if (x_130) {
    const float4 x_135 = asfloat(x_10[2]);
    const float4 x_137 = asfloat(x_10[3]);
    x_140 = all((lerp(x_135, x_137, (0.5f).xxxx) == float4(0.5f, 0.5f, 0.5f, 1.0f)));
    x_141 = x_140;
  }
  x_145 = false;
  if (x_141) {
    x_145 = true;
  }
  x_151 = false;
  if (x_145) {
    x_150 = true;
    x_151 = x_150;
  }
  x_157 = false;
  if (x_151) {
    x_156 = true;
    x_157 = x_156;
  }
  x_160 = false;
  if (x_157) {
    x_160 = true;
  }
  x_172 = false;
  if (x_160) {
    const float4 x_165 = asfloat(x_10[3]);
    const float4 x_169 = asfloat(x_10[4]);
    x_171 = (lerp(x_165.x, x_169.x, 0.0f) == 1.0f);
    x_172 = x_171;
  }
  x_186 = false;
  if (x_172) {
    const float4 x_177 = asfloat(x_10[3]);
    const float4 x_180 = asfloat(x_10[4]);
    x_185 = all((lerp(float2(x_177.x, x_177.y), float2(x_180.x, x_180.y), float2(0.0f, 0.5f)) == float2(1.0f, 0.5f)));
    x_186 = x_185;
  }
  x_200 = false;
  if (x_186) {
    const float4 x_191 = asfloat(x_10[3]);
    const float4 x_194 = asfloat(x_10[4]);
    x_199 = all((lerp(float3(x_191.x, x_191.y, x_191.z), float3(x_194.x, x_194.y, x_194.z), float3(0.0f, 0.5f, 0.0f)) == float3(1.0f, 0.5f, 1.0f)));
    x_200 = x_199;
  }
  x_211 = false;
  if (x_200) {
    const float4 x_205 = asfloat(x_10[3]);
    const float4 x_207 = asfloat(x_10[4]);
    x_210 = all((lerp(x_205, x_207, float4(0.0f, 0.5f, 0.0f, 1.0f)) == float4(1.0f, 0.5f, 1.0f, 2.25f)));
    x_211 = x_210;
  }
  x_214 = false;
  if (x_211) {
    x_214 = true;
  }
  x_221 = false;
  if (x_214) {
    x_220 = true;
    x_221 = x_220;
  }
  x_228 = false;
  if (x_221) {
    x_227 = true;
    x_228 = x_227;
  }
  x_231 = false;
  if (x_228) {
    x_231 = true;
  }
  if (x_231) {
    const float4 x_237 = asfloat(x_10[0]);
    x_232 = x_237;
  } else {
    const float4 x_239 = asfloat(x_10[1]);
    x_232 = x_239;
  }
  const float4 x_240 = x_232;
  return x_240;
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
