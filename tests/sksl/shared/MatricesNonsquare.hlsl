static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

bool test_half_b() {
  bool ok = false;
  float2x3 m23 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x4 m24 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x2 m32 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x4 m34 = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x2 m42 = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m22 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 m33 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  bool x_45 = false;
  bool x_46 = false;
  bool x_61 = false;
  bool x_62 = false;
  bool x_80 = false;
  bool x_81 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_120 = false;
  bool x_121 = false;
  bool x_143 = false;
  bool x_144 = false;
  bool x_161 = false;
  bool x_162 = false;
  bool x_184 = false;
  bool x_185 = false;
  bool x_201 = false;
  bool x_202 = false;
  bool x_223 = false;
  bool x_224 = false;
  bool x_240 = false;
  bool x_241 = false;
  ok = true;
  m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
  x_46 = false;
  if (true) {
    x_45 = true;
    x_46 = x_45;
  }
  ok = x_46;
  m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
  x_62 = false;
  if (x_46) {
    x_61 = true;
    x_62 = x_61;
  }
  ok = x_62;
  m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), (0.0f).xx);
  x_81 = false;
  if (x_62) {
    x_80 = true;
    x_81 = x_80;
  }
  ok = x_81;
  m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
  x_100 = false;
  if (x_81) {
    x_99 = true;
    x_100 = x_99;
  }
  ok = x_100;
  m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), (0.0f).xx, (0.0f).xx);
  x_121 = false;
  if (x_100) {
    x_120 = true;
    x_121 = x_120;
  }
  ok = x_121;
  m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), (0.0f).xxx);
  x_144 = false;
  if (x_121) {
    x_143 = true;
    x_144 = x_143;
  }
  ok = x_144;
  const float2x2 x_148 = float2x2(float2(8.0f, 0.0f), float2(0.0f, 8.0f));
  m22 = x_148;
  x_162 = false;
  if (x_144) {
    x_161 = (all((x_148[0u] == float2(8.0f, 0.0f))) & all((x_148[1u] == float2(0.0f, 8.0f))));
    x_162 = x_161;
  }
  ok = x_162;
  const float3x3 x_166 = float3x3(float3(35.0f, 0.0f, 0.0f), float3(0.0f, 35.0f, 0.0f), float3(0.0f, 0.0f, 35.0f));
  m33 = x_166;
  x_185 = false;
  if (x_162) {
    x_184 = ((all((x_166[0u] == float3(35.0f, 0.0f, 0.0f))) & all((x_166[1u] == float3(0.0f, 35.0f, 0.0f)))) & all((x_166[2u] == float3(0.0f, 0.0f, 35.0f))));
    x_185 = x_184;
  }
  ok = x_185;
  const float3 x_189 = float3(3.0f, 1.0f, 1.0f);
  const float3 x_190 = float3(1.0f, 3.0f, 1.0f);
  m23 = float2x3(x_189, x_190);
  x_202 = false;
  if (x_185) {
    x_201 = (all((x_189 == float3(3.0f, 1.0f, 1.0f))) & all((x_190 == float3(1.0f, 3.0f, 1.0f))));
    x_202 = x_201;
  }
  ok = x_202;
  const float2 x_205 = float2(2.0f, -2.0f);
  const float2 x_206 = float2(-2.0f, 2.0f);
  const float2 x_207 = (-2.0f).xx;
  m32 = float3x2(x_205, x_206, x_207);
  x_224 = false;
  if (x_202) {
    x_223 = ((all((x_205 == float2(2.0f, -2.0f))) & all((x_206 == float2(-2.0f, 2.0f)))) & all((x_207 == (-2.0f).xx)));
    x_224 = x_223;
  }
  ok = x_224;
  const float4 x_227 = float4(0.75f, 0.0f, 0.0f, 0.0f);
  const float4 x_228 = float4(0.0f, 0.75f, 0.0f, 0.0f);
  m24 = float2x4(x_227, x_228);
  x_241 = false;
  if (x_224) {
    x_240 = (all((x_227 == float4(0.75f, 0.0f, 0.0f, 0.0f))) & all((x_228 == float4(0.0f, 0.75f, 0.0f, 0.0f))));
    x_241 = x_240;
  }
  ok = x_241;
  return x_241;
}

float4 main(inout float2 x_243) {
  bool x_0_ok = false;
  float2x3 x_1_m23 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x4 x_2_m24 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x2 x_3_m32 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_7_m22 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_329 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_253 = false;
  bool x_254 = false;
  bool x_262 = false;
  bool x_263 = false;
  bool x_274 = false;
  bool x_275 = false;
  bool x_286 = false;
  bool x_287 = false;
  bool x_297 = false;
  bool x_298 = false;
  bool x_312 = false;
  bool x_313 = false;
  bool x_323 = false;
  bool x_324 = false;
  bool x_327 = false;
  bool x_328 = false;
  x_0_ok = true;
  x_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
  x_254 = false;
  if (true) {
    x_253 = true;
    x_254 = x_253;
  }
  x_0_ok = x_254;
  x_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
  x_263 = false;
  if (x_254) {
    x_262 = true;
    x_263 = x_262;
  }
  x_0_ok = x_263;
  x_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), (0.0f).xx);
  x_275 = false;
  if (x_263) {
    x_274 = true;
    x_275 = x_274;
  }
  x_0_ok = x_275;
  const float2x2 x_277 = float2x2(float2(8.0f, 0.0f), float2(0.0f, 8.0f));
  x_7_m22 = x_277;
  x_287 = false;
  if (x_275) {
    x_286 = (all((x_277[0u] == float2(8.0f, 0.0f))) & all((x_277[1u] == float2(0.0f, 8.0f))));
    x_287 = x_286;
  }
  x_0_ok = x_287;
  const float3 x_288 = float3(3.0f, 1.0f, 1.0f);
  const float3 x_289 = float3(1.0f, 3.0f, 1.0f);
  x_1_m23 = float2x3(x_288, x_289);
  x_298 = false;
  if (x_287) {
    x_297 = (all((x_288 == float3(3.0f, 1.0f, 1.0f))) & all((x_289 == float3(1.0f, 3.0f, 1.0f))));
    x_298 = x_297;
  }
  x_0_ok = x_298;
  const float2 x_299 = float2(2.0f, -2.0f);
  const float2 x_300 = float2(-2.0f, 2.0f);
  const float2 x_301 = (-2.0f).xx;
  x_3_m32 = float3x2(x_299, x_300, x_301);
  x_313 = false;
  if (x_298) {
    x_312 = ((all((x_299 == float2(2.0f, -2.0f))) & all((x_300 == float2(-2.0f, 2.0f)))) & all((x_301 == (-2.0f).xx)));
    x_313 = x_312;
  }
  x_0_ok = x_313;
  const float4 x_314 = float4(0.75f, 0.0f, 0.0f, 0.0f);
  const float4 x_315 = float4(0.0f, 0.75f, 0.0f, 0.0f);
  x_2_m24 = float2x4(x_314, x_315);
  x_324 = false;
  if (x_313) {
    x_323 = (all((x_314 == float4(0.75f, 0.0f, 0.0f, 0.0f))) & all((x_315 == float4(0.0f, 0.75f, 0.0f, 0.0f))));
    x_324 = x_323;
  }
  x_0_ok = x_324;
  x_328 = false;
  if (x_324) {
    x_327 = test_half_b();
    x_328 = x_327;
  }
  if (x_328) {
    const float4 x_338 = asfloat(x_11[0]);
    x_329 = x_338;
  } else {
    const float4 x_341 = asfloat(x_11[1]);
    x_329 = x_341;
  }
  const float4 x_342 = x_329;
  return x_342;
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
