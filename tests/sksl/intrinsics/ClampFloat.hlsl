static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expectedA = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedB = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_168 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_56 = false;
  bool x_57 = false;
  bool x_70 = false;
  bool x_71 = false;
  bool x_81 = false;
  bool x_82 = false;
  bool x_89 = false;
  bool x_90 = false;
  bool x_103 = false;
  bool x_104 = false;
  bool x_115 = false;
  bool x_116 = false;
  bool x_126 = false;
  bool x_127 = false;
  bool x_131 = false;
  bool x_137 = false;
  bool x_138 = false;
  bool x_144 = false;
  bool x_145 = false;
  bool x_148 = false;
  bool x_151 = false;
  bool x_156 = false;
  bool x_157 = false;
  bool x_163 = false;
  bool x_164 = false;
  bool x_167 = false;
  expectedA = float4(-1.0f, 0.0f, 0.75f, 1.0f);
  expectedB = float4(-1.0f, 0.0f, 0.5f, 2.25f);
  const float4 x_42 = asfloat(x_10[0]);
  x_57 = false;
  if ((clamp(x_42.x, -1.0f, 1.0f) == -1.0f)) {
    const float4 x_49 = asfloat(x_10[0]);
    x_56 = all((clamp(float2(x_49.x, x_49.y), (-1.0f).xx, (1.0f).xx) == float2(-1.0f, 0.0f)));
    x_57 = x_56;
  }
  x_71 = false;
  if (x_57) {
    const float4 x_62 = asfloat(x_10[0]);
    x_70 = all((clamp(float3(x_62.x, x_62.y, x_62.z), (-1.0f).xxx, (1.0f).xxx) == float3(-1.0f, 0.0f, 0.75f)));
    x_71 = x_70;
  }
  x_82 = false;
  if (x_71) {
    const float4 x_76 = asfloat(x_10[0]);
    x_81 = all((clamp(x_76, (-1.0f).xxxx, (1.0f).xxxx) == float4(-1.0f, 0.0f, 0.75f, 1.0f)));
    x_82 = x_81;
  }
  x_90 = false;
  if (x_82) {
    const float4 x_87 = asfloat(x_10[0]);
    x_89 = (clamp(x_87.x, -1.0f, 1.0f) == -1.0f);
    x_90 = x_89;
  }
  x_104 = false;
  if (x_90) {
    const float4 x_95 = asfloat(x_10[0]);
    x_103 = all((clamp(float2(x_95.x, x_95.y), float2(-1.0f, -2.0f), float2(1.0f, 2.0f)) == float2(-1.0f, 0.0f)));
    x_104 = x_103;
  }
  x_116 = false;
  if (x_104) {
    const float4 x_109 = asfloat(x_10[0]);
    x_115 = all((clamp(float3(x_109.x, x_109.y, x_109.z), float3(-1.0f, -2.0f, -2.0f), float3(1.0f, 2.0f, 0.5f)) == float3(-1.0f, 0.0f, 0.5f)));
    x_116 = x_115;
  }
  x_127 = false;
  if (x_116) {
    const float4 x_121 = asfloat(x_10[0]);
    x_126 = all((clamp(x_121, float4(-1.0f, -2.0f, -2.0f, 1.0f), float4(1.0f, 2.0f, 0.5f, 3.0f)) == float4(-1.0f, 0.0f, 0.5f, 2.25f)));
    x_127 = x_126;
  }
  x_131 = false;
  if (x_127) {
    x_131 = true;
  }
  x_138 = false;
  if (x_131) {
    x_137 = true;
    x_138 = x_137;
  }
  x_145 = false;
  if (x_138) {
    x_144 = true;
    x_145 = x_144;
  }
  x_148 = false;
  if (x_145) {
    x_148 = true;
  }
  x_151 = false;
  if (x_148) {
    x_151 = true;
  }
  x_157 = false;
  if (x_151) {
    x_156 = true;
    x_157 = x_156;
  }
  x_164 = false;
  if (x_157) {
    x_163 = true;
    x_164 = x_163;
  }
  x_167 = false;
  if (x_164) {
    x_167 = true;
  }
  if (x_167) {
    const float4 x_174 = asfloat(x_10[1]);
    x_168 = x_174;
  } else {
    const float4 x_177 = asfloat(x_10[2]);
    x_168 = x_177;
  }
  const float4 x_178 = x_168;
  return x_178;
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
