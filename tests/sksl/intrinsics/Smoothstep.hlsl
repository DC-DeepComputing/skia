static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expectedA = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedB = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_205 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_40 = false;
  bool x_41 = false;
  bool x_49 = false;
  bool x_50 = false;
  bool x_53 = false;
  bool x_56 = false;
  bool x_61 = false;
  bool x_62 = false;
  bool x_67 = false;
  bool x_68 = false;
  bool x_71 = false;
  bool x_86 = false;
  bool x_87 = false;
  bool x_102 = false;
  bool x_103 = false;
  bool x_119 = false;
  bool x_120 = false;
  bool x_136 = false;
  bool x_137 = false;
  bool x_140 = false;
  bool x_146 = false;
  bool x_147 = false;
  bool x_153 = false;
  bool x_154 = false;
  bool x_157 = false;
  bool x_167 = false;
  bool x_168 = false;
  bool x_180 = false;
  bool x_181 = false;
  bool x_193 = false;
  bool x_194 = false;
  bool x_203 = false;
  bool x_204 = false;
  expectedA = float4(0.0f, 0.0f, 0.84375f, 1.0f);
  expectedB = float4(1.0f, 0.0f, 1.0f, 1.0f);
  x_41 = false;
  if (true) {
    x_40 = true;
    x_41 = x_40;
  }
  x_50 = false;
  if (x_41) {
    x_49 = true;
    x_50 = x_49;
  }
  x_53 = false;
  if (x_50) {
    x_53 = true;
  }
  x_56 = false;
  if (x_53) {
    x_56 = true;
  }
  x_62 = false;
  if (x_56) {
    x_61 = true;
    x_62 = x_61;
  }
  x_68 = false;
  if (x_62) {
    x_67 = true;
    x_68 = x_67;
  }
  x_71 = false;
  if (x_68) {
    x_71 = true;
  }
  x_87 = false;
  if (x_71) {
    const float4 x_79 = asfloat(x_10[2]);
    const float4 x_83 = asfloat(x_10[1]);
    x_86 = (smoothstep(x_79.y, x_83.y, -1.25f) == 0.0f);
    x_87 = x_86;
  }
  x_103 = false;
  if (x_87) {
    const float4 x_92 = asfloat(x_10[2]);
    const float x_93 = x_92.y;
    const float4 x_96 = asfloat(x_10[1]);
    const float x_97 = x_96.y;
    x_102 = all((smoothstep(float2(x_93, x_93), float2(x_97, x_97), float2(-1.25f, 0.0f)) == (0.0f).xx));
    x_103 = x_102;
  }
  x_120 = false;
  if (x_103) {
    const float4 x_108 = asfloat(x_10[2]);
    const float x_109 = x_108.y;
    const float4 x_112 = asfloat(x_10[1]);
    const float x_113 = x_112.y;
    x_119 = all((smoothstep(float3(x_109, x_109, x_109), float3(x_113, x_113, x_113), float3(-1.25f, 0.0f, 0.75f)) == float3(0.0f, 0.0f, 0.84375f)));
    x_120 = x_119;
  }
  x_137 = false;
  if (x_120) {
    const float4 x_125 = asfloat(x_10[2]);
    const float x_126 = x_125.y;
    const float4 x_129 = asfloat(x_10[1]);
    const float x_130 = x_129.y;
    x_136 = all((smoothstep(float4(x_126, x_126, x_126, x_126), float4(x_130, x_130, x_130, x_130), float4(-1.25f, 0.0f, 0.75f, 2.25f)) == float4(0.0f, 0.0f, 0.84375f, 1.0f)));
    x_137 = x_136;
  }
  x_140 = false;
  if (x_137) {
    x_140 = true;
  }
  x_147 = false;
  if (x_140) {
    x_146 = true;
    x_147 = x_146;
  }
  x_154 = false;
  if (x_147) {
    x_153 = true;
    x_154 = x_153;
  }
  x_157 = false;
  if (x_154) {
    x_157 = true;
  }
  x_168 = false;
  if (x_157) {
    const float4 x_162 = asfloat(x_10[2]);
    const float4 x_165 = asfloat(x_10[1]);
    x_167 = (smoothstep(x_162.x, x_165.x, -1.25f) == 1.0f);
    x_168 = x_167;
  }
  x_181 = false;
  if (x_168) {
    const float4 x_173 = asfloat(x_10[2]);
    const float4 x_176 = asfloat(x_10[1]);
    x_180 = all((smoothstep(float2(x_173.x, x_173.y), float2(x_176.x, x_176.y), float2(-1.25f, 0.0f)) == float2(1.0f, 0.0f)));
    x_181 = x_180;
  }
  x_194 = false;
  if (x_181) {
    const float4 x_186 = asfloat(x_10[2]);
    const float4 x_189 = asfloat(x_10[1]);
    x_193 = all((smoothstep(float3(x_186.x, x_186.y, x_186.z), float3(x_189.x, x_189.y, x_189.z), float3(-1.25f, 0.0f, 0.75f)) == float3(1.0f, 0.0f, 1.0f)));
    x_194 = x_193;
  }
  x_204 = false;
  if (x_194) {
    const float4 x_199 = asfloat(x_10[2]);
    const float4 x_201 = asfloat(x_10[1]);
    x_203 = all((smoothstep(x_199, x_201, float4(-1.25f, 0.0f, 0.75f, 2.25f)) == float4(1.0f, 0.0f, 1.0f, 1.0f)));
    x_204 = x_203;
  }
  if (x_204) {
    const float4 x_210 = asfloat(x_10[1]);
    x_205 = x_210;
  } else {
    const float4 x_212 = asfloat(x_10[2]);
    x_205 = x_212;
  }
  const float4 x_213 = x_205;
  return x_213;
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
