uint tint_clamp(uint e, uint low, uint high) {
  return min(max(e, low), high);
}

uint2 tint_clamp_1(uint2 e, uint2 low, uint2 high) {
  return min(max(e, low), high);
}

uint3 tint_clamp_2(uint3 e, uint3 low, uint3 high) {
  return min(max(e, low), high);
}

uint4 tint_clamp_3(uint4 e, uint4 low, uint4 high) {
  return min(max(e, low), high);
}

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  uint4 uintValues = uint4(0u, 0u, 0u, 0u);
  uint4 expectedA = uint4(0u, 0u, 0u, 0u);
  uint4 expectedB = uint4(0u, 0u, 0u, 0u);
  float4 x_172 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_73 = false;
  bool x_74 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_94 = false;
  bool x_95 = false;
  bool x_99 = false;
  bool x_105 = false;
  bool x_106 = false;
  bool x_112 = false;
  bool x_113 = false;
  bool x_116 = false;
  bool x_120 = false;
  bool x_121 = false;
  bool x_132 = false;
  bool x_133 = false;
  bool x_142 = false;
  bool x_143 = false;
  bool x_151 = false;
  bool x_152 = false;
  bool x_155 = false;
  bool x_160 = false;
  bool x_161 = false;
  bool x_167 = false;
  bool x_168 = false;
  bool x_171 = false;
  const float4 x_34 = asfloat(x_10[0]);
  const float4 x_39 = ((x_34 * 100.0f) + (200.0f).xxxx);
  const uint4 x_48 = uint4(uint(x_39.x), uint(x_39.y), uint(x_39.z), uint(x_39.w));
  uintValues = x_48;
  expectedA = uint4(100u, 200u, 275u, 300u);
  expectedB = uint4(100u, 200u, 250u, 425u);
  const uint x_61 = x_48.x;
  x_74 = false;
  const uint tint_symbol_3 = tint_clamp(x_61, 100u, 300u);
  if ((tint_symbol_3 == 100u)) {
    const uint2 tint_symbol_4 = tint_clamp_1(uint2(x_48.x, x_48.y), (100u).xx, (300u).xx);
    x_73 = all((tint_symbol_4 == uint2(100u, 200u)));
    x_74 = x_73;
  }
  x_86 = false;
  if (x_74) {
    const uint3 tint_symbol_5 = tint_clamp_2(uint3(x_48.x, x_48.y, x_48.z), (100u).xxx, (300u).xxx);
    x_85 = all((tint_symbol_5 == uint3(100u, 200u, 275u)));
    x_86 = x_85;
  }
  x_95 = false;
  if (x_86) {
    const uint4 tint_symbol_6 = tint_clamp_3(x_48, (100u).xxxx, (300u).xxxx);
    x_94 = all((tint_symbol_6 == uint4(100u, 200u, 275u, 300u)));
    x_95 = x_94;
  }
  x_99 = false;
  if (x_95) {
    x_99 = true;
  }
  x_106 = false;
  if (x_99) {
    x_105 = true;
    x_106 = x_105;
  }
  x_113 = false;
  if (x_106) {
    x_112 = true;
    x_113 = x_112;
  }
  x_116 = false;
  if (x_113) {
    x_116 = true;
  }
  x_121 = false;
  if (x_116) {
    const uint tint_symbol_7 = tint_clamp(x_61, 100u, 300u);
    x_120 = (tint_symbol_7 == 100u);
    x_121 = x_120;
  }
  x_133 = false;
  if (x_121) {
    const uint2 tint_symbol_8 = tint_clamp_1(uint2(x_48.x, x_48.y), uint2(100u, 0u), uint2(300u, 400u));
    x_132 = all((tint_symbol_8 == uint2(100u, 200u)));
    x_133 = x_132;
  }
  x_143 = false;
  if (x_133) {
    const uint3 tint_symbol_9 = tint_clamp_2(uint3(x_48.x, x_48.y, x_48.z), uint3(100u, 0u, 0u), uint3(300u, 400u, 250u));
    x_142 = all((tint_symbol_9 == uint3(100u, 200u, 250u)));
    x_143 = x_142;
  }
  x_152 = false;
  if (x_143) {
    const uint4 tint_symbol_10 = tint_clamp_3(x_48, uint4(100u, 0u, 0u, 300u), uint4(300u, 400u, 250u, 500u));
    x_151 = all((tint_symbol_10 == uint4(100u, 200u, 250u, 425u)));
    x_152 = x_151;
  }
  x_155 = false;
  if (x_152) {
    x_155 = true;
  }
  x_161 = false;
  if (x_155) {
    x_160 = true;
    x_161 = x_160;
  }
  x_168 = false;
  if (x_161) {
    x_167 = true;
    x_168 = x_167;
  }
  x_171 = false;
  if (x_168) {
    x_171 = true;
  }
  if (x_171) {
    const float4 x_179 = asfloat(x_10[1]);
    x_172 = x_179;
  } else {
    const float4 x_182 = asfloat(x_10[2]);
    x_172 = x_182;
  }
  const float4 x_183 = x_172;
  return x_183;
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
  const _entrypoint_out tint_symbol_12 = {sk_FragColor};
  return tint_symbol_12;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
