static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expectedA = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedB = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_167 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_54 = false;
  bool x_55 = false;
  bool x_67 = false;
  bool x_68 = false;
  bool x_77 = false;
  bool x_78 = false;
  bool x_82 = false;
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
  expectedA = float4(0.5f, 0.5f, 0.75f, 2.25f);
  expectedB = float4(0.0f, 1.0f, 0.75f, 2.25f);
  const float4 x_41 = asfloat(x_10[0]);
  x_55 = false;
  if ((max(x_41.x, 0.5f) == 0.5f)) {
    const float4 x_48 = asfloat(x_10[0]);
    x_54 = all((max(float2(x_48.x, x_48.y), (0.5f).xx) == (0.5f).xx));
    x_55 = x_54;
  }
  x_68 = false;
  if (x_55) {
    const float4 x_60 = asfloat(x_10[0]);
    x_67 = all((max(float3(x_60.x, x_60.y, x_60.z), (0.5f).xxx) == float3(0.5f, 0.5f, 0.75f)));
    x_68 = x_67;
  }
  x_78 = false;
  if (x_68) {
    const float4 x_73 = asfloat(x_10[0]);
    x_77 = all((max(x_73, (0.5f).xxxx) == float4(0.5f, 0.5f, 0.75f, 2.25f)));
    x_78 = x_77;
  }
  x_82 = false;
  if (x_78) {
    x_82 = true;
  }
  x_88 = false;
  if (x_82) {
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
    const float4 x_107 = asfloat(x_10[1]);
    x_109 = (max(x_103.x, x_107.x) == 0.0f);
    x_110 = x_109;
  }
  x_123 = false;
  if (x_110) {
    const float4 x_115 = asfloat(x_10[0]);
    const float4 x_118 = asfloat(x_10[1]);
    x_122 = all((max(float2(x_115.x, x_115.y), float2(x_118.x, x_118.y)) == float2(0.0f, 1.0f)));
    x_123 = x_122;
  }
  x_136 = false;
  if (x_123) {
    const float4 x_128 = asfloat(x_10[0]);
    const float4 x_131 = asfloat(x_10[1]);
    x_135 = all((max(float3(x_128.x, x_128.y, x_128.z), float3(x_131.x, x_131.y, x_131.z)) == float3(0.0f, 1.0f, 0.75f)));
    x_136 = x_135;
  }
  x_146 = false;
  if (x_136) {
    const float4 x_141 = asfloat(x_10[0]);
    const float4 x_143 = asfloat(x_10[1]);
    x_145 = all((max(x_141, x_143) == float4(0.0f, 1.0f, 0.75f, 2.25f)));
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
    const float4 x_172 = asfloat(x_10[1]);
    x_167 = x_172;
  } else {
    const float4 x_175 = asfloat(x_10[2]);
    x_167 = x_175;
  }
  const float4 x_176 = x_167;
  return x_176;
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
