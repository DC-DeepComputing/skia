static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

bool test_half_b() {
  bool ok = false;
  float2x2 m1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m3 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m4 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m5 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m7 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 m9 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 m10 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 m11 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  bool x_48 = false;
  bool x_49 = false;
  bool x_57 = false;
  bool x_58 = false;
  bool x_70 = false;
  bool x_71 = false;
  bool x_87 = false;
  bool x_88 = false;
  bool x_107 = false;
  bool x_108 = false;
  bool x_123 = false;
  bool x_124 = false;
  bool x_136 = false;
  bool x_137 = false;
  bool x_157 = false;
  bool x_158 = false;
  bool x_181 = false;
  bool x_182 = false;
  bool x_209 = false;
  bool x_210 = false;
  ok = true;
  m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  x_49 = false;
  if (true) {
    x_48 = true;
    x_49 = x_48;
  }
  ok = x_49;
  m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  x_58 = false;
  if (x_49) {
    x_57 = true;
    x_58 = x_57;
  }
  ok = x_58;
  m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
  x_71 = false;
  if (x_58) {
    x_70 = true;
    x_71 = x_70;
  }
  ok = x_71;
  const float2x2 x_72 = float2x2(float2(6.0f, 12.0f), float2(18.0f, 24.0f));
  m3 = x_72;
  x_88 = false;
  if (x_71) {
    x_87 = (all((x_72[0u] == float2(6.0f, 12.0f))) & all((x_72[1u] == float2(18.0f, 24.0f))));
    x_88 = x_87;
  }
  ok = x_88;
  const float2 x_93 = m1[1];
  const float x_94 = x_93.y;
  const float2 x_95 = float2(x_94, 0.0f);
  const float2 x_96 = float2(0.0f, x_94);
  m5 = float2x2(x_95, x_96);
  x_108 = false;
  if (x_88) {
    x_107 = (all((x_95 == float2(4.0f, 0.0f))) & all((x_96 == float2(0.0f, 4.0f))));
    x_108 = x_107;
  }
  ok = x_108;
  const float2 x_109 = (float2(1.0f, 2.0f) + x_95);
  const float2 x_110 = (float2(3.0f, 4.0f) + x_96);
  m1 = float2x2(x_109, x_110);
  x_124 = false;
  if (x_108) {
    x_123 = (all((x_109 == float2(5.0f, 2.0f))) & all((x_110 == float2(3.0f, 8.0f))));
    x_124 = x_123;
  }
  ok = x_124;
  m7 = float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f));
  x_137 = false;
  if (x_124) {
    x_136 = true;
    x_137 = x_136;
  }
  ok = x_137;
  m9 = float3x3(float3(9.0f, 0.0f, 0.0f), float3(0.0f, 9.0f, 0.0f), float3(0.0f, 0.0f, 9.0f));
  x_158 = false;
  if (x_137) {
    x_157 = true;
    x_158 = x_157;
  }
  ok = x_158;
  m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
  x_182 = false;
  if (x_158) {
    x_181 = true;
    x_182 = x_181;
  }
  ok = x_182;
  m11 = float4x4((20.0f).xxxx, (20.0f).xxxx, (20.0f).xxxx, (20.0f).xxxx);
  const float4 x_187 = float4(9.0f, 20.0f, 20.0f, 20.0f);
  const float4 x_188 = float4(20.0f, 9.0f, 20.0f, 20.0f);
  const float4 x_189 = float4(20.0f, 20.0f, 9.0f, 20.0f);
  const float4 x_190 = float4(20.0f, 20.0f, 20.0f, 9.0f);
  m11 = float4x4(x_187, x_188, x_189, x_190);
  x_210 = false;
  if (x_182) {
    x_209 = (((all((x_187 == float4(9.0f, 20.0f, 20.0f, 20.0f))) & all((x_188 == float4(20.0f, 9.0f, 20.0f, 20.0f)))) & all((x_189 == float4(20.0f, 20.0f, 9.0f, 20.0f)))) & all((x_190 == float4(20.0f, 20.0f, 20.0f, 9.0f))));
    x_210 = x_209;
  }
  ok = x_210;
  return x_210;
}

bool test_comma_b() {
  float2x2 x = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 y = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  y = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  return true;
}

float4 main(inout float2 x_220) {
  bool x_0_ok = false;
  float2x2 x_1_m1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_2_m3 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_3_m4 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_4_m5 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_7_m10 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_8_m11 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_308 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_230 = false;
  bool x_231 = false;
  bool x_239 = false;
  bool x_240 = false;
  bool x_251 = false;
  bool x_252 = false;
  bool x_266 = false;
  bool x_267 = false;
  bool x_277 = false;
  bool x_278 = false;
  bool x_298 = false;
  bool x_299 = false;
  bool x_302 = false;
  bool x_303 = false;
  bool x_306 = false;
  bool x_307 = false;
  x_0_ok = true;
  x_1_m1 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  x_231 = false;
  if (true) {
    x_230 = true;
    x_231 = x_230;
  }
  x_0_ok = x_231;
  x_2_m3 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  x_240 = false;
  if (x_231) {
    x_239 = true;
    x_240 = x_239;
  }
  x_0_ok = x_240;
  x_3_m4 = float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f));
  const float2x2 x_242 = float2x2(float2(6.0f, 12.0f), float2(18.0f, 24.0f));
  x_2_m3 = x_242;
  x_252 = false;
  if (x_240) {
    x_251 = (all((x_242[0u] == float2(6.0f, 12.0f))) & all((x_242[1u] == float2(18.0f, 24.0f))));
    x_252 = x_251;
  }
  x_0_ok = x_252;
  const float2 x_255 = x_1_m1[1];
  const float x_256 = x_255.y;
  const float2 x_257 = float2(x_256, 0.0f);
  const float2 x_258 = float2(0.0f, x_256);
  x_4_m5 = float2x2(x_257, x_258);
  x_267 = false;
  if (x_252) {
    x_266 = (all((x_257 == float2(4.0f, 0.0f))) & all((x_258 == float2(0.0f, 4.0f))));
    x_267 = x_266;
  }
  x_0_ok = x_267;
  const float2 x_268 = (float2(1.0f, 2.0f) + x_257);
  const float2 x_269 = (float2(3.0f, 4.0f) + x_258);
  x_1_m1 = float2x2(x_268, x_269);
  x_278 = false;
  if (x_267) {
    x_277 = (all((x_268 == float2(5.0f, 2.0f))) & all((x_269 == float2(3.0f, 8.0f))));
    x_278 = x_277;
  }
  x_0_ok = x_278;
  x_7_m10 = float4x4(float4(11.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 11.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 11.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 11.0f));
  x_8_m11 = float4x4((20.0f).xxxx, (20.0f).xxxx, (20.0f).xxxx, (20.0f).xxxx);
  const float4 x_281 = float4(9.0f, 20.0f, 20.0f, 20.0f);
  const float4 x_282 = float4(20.0f, 9.0f, 20.0f, 20.0f);
  const float4 x_283 = float4(20.0f, 20.0f, 9.0f, 20.0f);
  const float4 x_284 = float4(20.0f, 20.0f, 20.0f, 9.0f);
  x_8_m11 = float4x4(x_281, x_282, x_283, x_284);
  x_299 = false;
  if (x_278) {
    x_298 = (((all((x_281 == float4(9.0f, 20.0f, 20.0f, 20.0f))) & all((x_282 == float4(20.0f, 9.0f, 20.0f, 20.0f)))) & all((x_283 == float4(20.0f, 20.0f, 9.0f, 20.0f)))) & all((x_284 == float4(20.0f, 20.0f, 20.0f, 9.0f))));
    x_299 = x_298;
  }
  x_0_ok = x_299;
  x_303 = false;
  if (x_299) {
    x_302 = test_half_b();
    x_303 = x_302;
  }
  x_307 = false;
  if (x_303) {
    x_306 = test_comma_b();
    x_307 = x_306;
  }
  if (x_307) {
    const float4 x_316 = asfloat(x_12[0]);
    x_308 = x_316;
  } else {
    const float4 x_318 = asfloat(x_12[1]);
    x_308 = x_318;
  }
  const float4 x_319 = x_308;
  return x_319;
}

void _entrypoint_1() {
  float2 x_22 = float2(0.0f, 0.0f);
  x_22 = (0.0f).xx;
  const float4 x_24 = main(x_22);
  sk_FragColor = x_24;
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
