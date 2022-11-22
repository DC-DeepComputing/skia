static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

bool test_bifffff22(inout int x_30, inout float x_31, inout float x_32, inout float x_33, inout float x_34, inout float2x2 x_35) {
  float one = 0.0f;
  float2x2 m2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_101 = false;
  bool x_102 = false;
  bool x_111 = false;
  bool x_112 = false;
  bool x_121 = false;
  bool x_122 = false;
  const float4 x_41 = asfloat(x_11[1]);
  const float x_42 = x_41.x;
  one = x_42;
  const float x_44 = x_31;
  const float x_46 = x_32;
  const float x_48 = x_33;
  const float x_50 = x_34;
  const float2 x_52 = float2((x_44 * x_42), (x_46 * x_42));
  const float2 x_53 = float2((x_48 * x_42), (x_50 * x_42));
  m2 = float2x2(x_52, x_53);
  const int x_55 = x_30;
  switch(x_55) {
    case 4: {
      const float2x2 x_76 = m2;
      m2 = float2x2((x_76[0u] / (2.0f).xx), (x_76[1u] / (2.0f).xx));
      break;
    }
    case 3: {
      const float2x2 x_73 = m2;
      m2 = (x_73 * 2.0f);
      break;
    }
    case 2: {
      const float2x2 x_67 = m2;
      m2 = float2x2((x_67[0u] - (1.0f).xx), (x_67[1u] - (1.0f).xx));
      break;
    }
    case 1: {
      m2 = float2x2((x_52 + (1.0f).xx), (x_53 + (1.0f).xx));
      break;
    }
    default: {
      break;
    }
  }
  const float2 x_87 = m2[0];
  const float2 x_90 = x_35[0];
  x_102 = false;
  if ((x_87.x == x_90.x)) {
    const float2 x_96 = m2[0];
    const float2 x_99 = x_35[0];
    x_101 = (x_96.y == x_99.y);
    x_102 = x_101;
  }
  x_112 = false;
  if (x_102) {
    const float2 x_106 = m2[1];
    const float2 x_109 = x_35[1];
    x_111 = (x_106.x == x_109.x);
    x_112 = x_111;
  }
  x_122 = false;
  if (x_112) {
    const float2 x_116 = m2[1];
    const float2 x_119 = x_35[1];
    x_121 = (x_116.y == x_119.y);
    x_122 = x_121;
  }
  return x_122;
}

float4 main(inout float2 x_124) {
  float f1 = 0.0f;
  float f2 = 0.0f;
  float f3 = 0.0f;
  float f4 = 0.0f;
  float2x2 x_0_expected = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float x_1_one = 0.0f;
  float2x2 x_2_m2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_210 = 0;
  float x_211 = 0.0f;
  float x_212 = 0.0f;
  float x_213 = 0.0f;
  float x_214 = 0.0f;
  float2x2 x_222 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_228 = 0;
  float x_229 = 0.0f;
  float x_230 = 0.0f;
  float x_231 = 0.0f;
  float x_232 = 0.0f;
  float2x2 x_240 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_246 = 0;
  float x_247 = 0.0f;
  float x_248 = 0.0f;
  float x_249 = 0.0f;
  float x_250 = 0.0f;
  float2x2 x_259 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_262 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_185 = false;
  bool x_186 = false;
  bool x_195 = false;
  bool x_196 = false;
  bool x_205 = false;
  bool x_206 = false;
  bool x_223 = false;
  bool x_224 = false;
  bool x_241 = false;
  bool x_242 = false;
  bool x_260 = false;
  bool x_261 = false;
  const float4 x_128 = asfloat(x_11[0]);
  const float x_129 = x_128.y;
  f1 = x_129;
  const float4 x_132 = asfloat(x_11[0]);
  const float x_134 = (2.0f * x_132.y);
  f2 = x_134;
  const float4 x_138 = asfloat(x_11[0]);
  const float x_140 = (3.0f * x_138.y);
  f3 = x_140;
  const float4 x_144 = asfloat(x_11[0]);
  const float x_146 = (4.0f * x_144.y);
  f4 = x_146;
  x_0_expected = float2x2(float2((x_129 + 1.0f), (x_134 + 1.0f)), float2((x_140 + 1.0f), (x_146 + 1.0f)));
  const float4 x_157 = asfloat(x_11[1]);
  const float x_158 = x_157.x;
  x_1_one = x_158;
  const float2 x_164 = float2((x_129 * x_158), (x_134 * x_158));
  const float2 x_165 = float2((x_140 * x_158), (x_146 * x_158));
  x_2_m2 = float2x2(x_164, x_165);
  x_2_m2 = float2x2((x_164 + (1.0f).xx), (x_165 + (1.0f).xx));
  const float2 x_171 = x_2_m2[0];
  const float2 x_174 = x_0_expected[0];
  x_186 = false;
  if ((x_171.x == x_174.x)) {
    const float2 x_180 = x_2_m2[0];
    const float2 x_183 = x_0_expected[0];
    x_185 = (x_180.y == x_183.y);
    x_186 = x_185;
  }
  x_196 = false;
  if (x_186) {
    const float2 x_190 = x_2_m2[1];
    const float2 x_193 = x_0_expected[1];
    x_195 = (x_190.x == x_193.x);
    x_196 = x_195;
  }
  x_206 = false;
  if (x_196) {
    const float2 x_200 = x_2_m2[1];
    const float2 x_203 = x_0_expected[1];
    x_205 = (x_200.y == x_203.y);
    x_206 = x_205;
  }
  x_224 = false;
  if (x_206) {
    x_210 = 2;
    x_211 = x_129;
    x_212 = x_134;
    x_213 = x_140;
    x_214 = x_146;
    x_222 = float2x2(float2((x_129 - 1.0f), (x_134 - 1.0f)), float2((x_140 - 1.0f), (x_146 - 1.0f)));
    x_223 = test_bifffff22(x_210, x_211, x_212, x_213, x_214, x_222);
    x_224 = x_223;
  }
  x_242 = false;
  if (x_224) {
    x_228 = 3;
    x_229 = x_129;
    x_230 = x_134;
    x_231 = x_140;
    x_232 = x_146;
    x_240 = float2x2(float2((x_129 * 2.0f), (x_134 * 2.0f)), float2((x_140 * 2.0f), (x_146 * 2.0f)));
    x_241 = test_bifffff22(x_228, x_229, x_230, x_231, x_232, x_240);
    x_242 = x_241;
  }
  x_261 = false;
  if (x_242) {
    x_246 = 4;
    x_247 = x_129;
    x_248 = x_134;
    x_249 = x_140;
    x_250 = x_146;
    x_259 = float2x2(float2((x_129 * 0.5f), (x_134 * 0.5f)), float2((x_140 * 0.5f), (x_146 * 0.5f)));
    x_260 = test_bifffff22(x_246, x_247, x_248, x_249, x_250, x_259);
    x_261 = x_260;
  }
  if (x_261) {
    const float4 x_268 = asfloat(x_11[0]);
    x_262 = x_268;
  } else {
    const float4 x_270 = asfloat(x_11[1]);
    x_262 = x_270;
  }
  const float4 x_271 = x_262;
  return x_271;
}

void _entrypoint_1() {
  float2 x_21 = float2(0.0f, 0.0f);
  x_21 = (0.0f).xx;
  const float4 x_23 = main(x_21);
  sk_FragColor = x_23;
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
