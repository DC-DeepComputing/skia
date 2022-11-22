static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

bool test_matrix_op_matrix_half_b() {
  bool ok = false;
  float3x2 splat_4 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x2 m = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 splat_4_1 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 m_1 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 m_2 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x2 m_3 = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x4 m_4 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 m_5 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  bool x_59 = false;
  bool x_60 = false;
  bool x_80 = false;
  bool x_81 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_124 = false;
  bool x_125 = false;
  bool x_138 = false;
  bool x_139 = false;
  bool x_151 = false;
  bool x_152 = false;
  bool x_199 = false;
  bool x_200 = false;
  bool x_250 = false;
  bool x_251 = false;
  bool x_273 = false;
  bool x_274 = false;
  bool x_300 = false;
  bool x_301 = false;
  ok = true;
  splat_4 = float3x2((4.0f).xx, (4.0f).xx, (4.0f).xx);
  m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_40 = float2(6.0f, 4.0f);
  const float2 x_41 = float2(4.0f, 6.0f);
  const float2 x_42 = (4.0f).xx;
  m = float3x2(x_40, x_41, x_42);
  x_60 = false;
  if (true) {
    x_59 = ((all((x_40 == float2(6.0f, 4.0f))) & all((x_41 == float2(4.0f, 6.0f)))) & all((x_42 == (4.0f).xx)));
    x_60 = x_59;
  }
  ok = x_60;
  m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_61 = float2(-2.0f, -4.0f);
  const float2 x_62 = float2(-4.0f, -2.0f);
  const float2 x_63 = (-4.0f).xx;
  m = float3x2(x_61, x_62, x_63);
  x_81 = false;
  if (x_60) {
    x_80 = ((all((x_61 == float2(-2.0f, -4.0f))) & all((x_62 == float2(-4.0f, -2.0f)))) & all((x_63 == (-4.0f).xx)));
    x_81 = x_80;
  }
  ok = x_81;
  m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_82 = float2(0.5f, 0.0f);
  const float2 x_83 = float2(0.0f, 0.5f);
  const float2 x_84 = (0.0f).xx;
  m = float3x2(x_82, x_83, x_84);
  x_100 = false;
  if (x_81) {
    x_99 = ((all((x_82 == float2(0.5f, 0.0f))) & all((x_83 == float2(0.0f, 0.5f)))) & all((x_84 == (0.0f).xx)));
    x_100 = x_99;
  }
  ok = x_100;
  splat_4_1 = float2x3((4.0f).xxx, (4.0f).xxx);
  m_1 = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_111 = float3(6.0f, 4.0f, 4.0f);
  const float3 x_112 = float3(4.0f, 6.0f, 4.0f);
  m_1 = float2x3(x_111, x_112);
  x_125 = false;
  if (x_100) {
    x_124 = (all((x_111 == float3(6.0f, 4.0f, 4.0f))) & all((x_112 == float3(4.0f, 6.0f, 4.0f))));
    x_125 = x_124;
  }
  ok = x_125;
  m_1 = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_126 = float3(2.0f, 4.0f, 4.0f);
  const float3 x_127 = float3(4.0f, 2.0f, 4.0f);
  m_1 = float2x3(x_126, x_127);
  x_139 = false;
  if (x_125) {
    x_138 = (all((x_126 == float3(2.0f, 4.0f, 4.0f))) & all((x_127 == float3(4.0f, 2.0f, 4.0f))));
    x_139 = x_138;
  }
  ok = x_139;
  m_1 = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_142 = (2.0f).xxx;
  const float3 x_143 = (2.0f).xxx;
  m_1 = float2x3(x_142, x_143);
  x_152 = false;
  if (x_139) {
    x_151 = (all((x_142 == (2.0f).xxx)) & all((x_143 == (2.0f).xxx)));
    x_152 = x_151;
  }
  ok = x_152;
  m_2 = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
  const float3 x_179 = (17.0f).xxx;
  const float3 x_180 = (17.0f).xxx;
  const float3 x_181 = (17.0f).xxx;
  const float3 x_182 = (17.0f).xxx;
  m_2 = float4x3(x_179, x_180, x_181, x_182);
  x_200 = false;
  if (x_152) {
    x_199 = (((all((x_179 == (17.0f).xxx)) & all((x_180 == (17.0f).xxx))) & all((x_181 == (17.0f).xxx))) & all((x_182 == (17.0f).xxx)));
    x_200 = x_199;
  }
  ok = x_200;
  m_3 = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
  const float2 x_221 = float2(9.0f, 18.0f);
  const float2 x_222 = float2(27.0f, 36.0f);
  const float2 x_223 = float2(45.0f, 54.0f);
  const float2 x_224 = float2(63.0f, 72.0f);
  m_3 = float4x2(x_221, x_222, x_223, x_224);
  x_251 = false;
  if (x_200) {
    x_250 = (((all((x_221 == float2(9.0f, 18.0f))) & all((x_222 == float2(27.0f, 36.0f)))) & all((x_223 == float2(45.0f, 54.0f)))) & all((x_224 == float2(63.0f, 72.0f))));
    x_251 = x_250;
  }
  ok = x_251;
  m_4 = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
  const float4 x_260 = float4(1.0f, 2.0f, 3.0f, 4.0f);
  const float4 x_261 = float4(2.0f, 4.0f, 6.0f, 8.0f);
  m_4 = float2x4(x_260, x_261);
  x_274 = false;
  if (x_251) {
    x_273 = (all((x_260 == float4(1.0f, 2.0f, 3.0f, 4.0f))) & all((x_261 == float4(2.0f, 4.0f, 6.0f, 8.0f))));
    x_274 = x_273;
  }
  ok = x_274;
  m_5 = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
  const float2x3 x_283 = float2x3(float3(39.0f, 49.0f, 59.0f), float3(54.0f, 68.0f, 82.0f));
  m_5 = x_283;
  x_301 = false;
  if (x_274) {
    x_300 = (all((x_283[0u] == float3(39.0f, 49.0f, 59.0f))) & all((x_283[1u] == float3(54.0f, 68.0f, 82.0f))));
    x_301 = x_300;
  }
  ok = x_301;
  return x_301;
}

float4 main(inout float2 x_303) {
  bool x_0_ok = false;
  float3x2 x_1_splat_4 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x2 x_2_m = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 x_3_splat_4 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 x_4_m = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 x_5_m = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x2 x_6_m = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x4 x_7_m = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 x_8_m = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_456 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_321 = false;
  bool x_322 = false;
  bool x_336 = false;
  bool x_337 = false;
  bool x_351 = false;
  bool x_352 = false;
  bool x_364 = false;
  bool x_365 = false;
  bool x_375 = false;
  bool x_376 = false;
  bool x_386 = false;
  bool x_387 = false;
  bool x_406 = false;
  bool x_407 = false;
  bool x_426 = false;
  bool x_427 = false;
  bool x_438 = false;
  bool x_439 = false;
  bool x_450 = false;
  bool x_451 = false;
  bool x_454 = false;
  bool x_455 = false;
  x_0_ok = true;
  x_1_splat_4 = float3x2((4.0f).xx, (4.0f).xx, (4.0f).xx);
  x_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_308 = float2(6.0f, 4.0f);
  const float2 x_309 = float2(4.0f, 6.0f);
  const float2 x_310 = (4.0f).xx;
  x_2_m = float3x2(x_308, x_309, x_310);
  x_322 = false;
  if (true) {
    x_321 = ((all((x_308 == float2(6.0f, 4.0f))) & all((x_309 == float2(4.0f, 6.0f)))) & all((x_310 == (4.0f).xx)));
    x_322 = x_321;
  }
  x_0_ok = x_322;
  x_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_323 = float2(-2.0f, -4.0f);
  const float2 x_324 = float2(-4.0f, -2.0f);
  const float2 x_325 = (-4.0f).xx;
  x_2_m = float3x2(x_323, x_324, x_325);
  x_337 = false;
  if (x_322) {
    x_336 = ((all((x_323 == float2(-2.0f, -4.0f))) & all((x_324 == float2(-4.0f, -2.0f)))) & all((x_325 == (-4.0f).xx)));
    x_337 = x_336;
  }
  x_0_ok = x_337;
  x_2_m = float3x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f), (0.0f).xx);
  const float2 x_338 = float2(0.5f, 0.0f);
  const float2 x_339 = float2(0.0f, 0.5f);
  const float2 x_340 = (0.0f).xx;
  x_2_m = float3x2(x_338, x_339, x_340);
  x_352 = false;
  if (x_337) {
    x_351 = ((all((x_338 == float2(0.5f, 0.0f))) & all((x_339 == float2(0.0f, 0.5f)))) & all((x_340 == (0.0f).xx)));
    x_352 = x_351;
  }
  x_0_ok = x_352;
  x_3_splat_4 = float2x3((4.0f).xxx, (4.0f).xxx);
  x_4_m = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_355 = float3(6.0f, 4.0f, 4.0f);
  const float3 x_356 = float3(4.0f, 6.0f, 4.0f);
  x_4_m = float2x3(x_355, x_356);
  x_365 = false;
  if (x_352) {
    x_364 = (all((x_355 == float3(6.0f, 4.0f, 4.0f))) & all((x_356 == float3(4.0f, 6.0f, 4.0f))));
    x_365 = x_364;
  }
  x_0_ok = x_365;
  x_4_m = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_366 = float3(2.0f, 4.0f, 4.0f);
  const float3 x_367 = float3(4.0f, 2.0f, 4.0f);
  x_4_m = float2x3(x_366, x_367);
  x_376 = false;
  if (x_365) {
    x_375 = (all((x_366 == float3(2.0f, 4.0f, 4.0f))) & all((x_367 == float3(4.0f, 2.0f, 4.0f))));
    x_376 = x_375;
  }
  x_0_ok = x_376;
  x_4_m = float2x3((4.0f).xxx, (4.0f).xxx);
  const float3 x_377 = (2.0f).xxx;
  const float3 x_378 = (2.0f).xxx;
  x_4_m = float2x3(x_377, x_378);
  x_387 = false;
  if (x_376) {
    x_386 = (all((x_377 == (2.0f).xxx)) & all((x_378 == (2.0f).xxx)));
    x_387 = x_386;
  }
  x_0_ok = x_387;
  x_5_m = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f), float3(10.0f, 11.0f, 12.0f));
  const float3 x_389 = (17.0f).xxx;
  const float3 x_390 = (17.0f).xxx;
  const float3 x_391 = (17.0f).xxx;
  const float3 x_392 = (17.0f).xxx;
  x_5_m = float4x3(x_389, x_390, x_391, x_392);
  x_407 = false;
  if (x_387) {
    x_406 = (((all((x_389 == (17.0f).xxx)) & all((x_390 == (17.0f).xxx))) & all((x_391 == (17.0f).xxx))) & all((x_392 == (17.0f).xxx)));
    x_407 = x_406;
  }
  x_0_ok = x_407;
  x_6_m = float4x2(float2(10.0f, 20.0f), float2(30.0f, 40.0f), float2(50.0f, 60.0f), float2(70.0f, 80.0f));
  const float2 x_409 = float2(9.0f, 18.0f);
  const float2 x_410 = float2(27.0f, 36.0f);
  const float2 x_411 = float2(45.0f, 54.0f);
  const float2 x_412 = float2(63.0f, 72.0f);
  x_6_m = float4x2(x_409, x_410, x_411, x_412);
  x_427 = false;
  if (x_407) {
    x_426 = (((all((x_409 == float2(9.0f, 18.0f))) & all((x_410 == float2(27.0f, 36.0f)))) & all((x_411 == float2(45.0f, 54.0f)))) & all((x_412 == float2(63.0f, 72.0f))));
    x_427 = x_426;
  }
  x_0_ok = x_427;
  x_7_m = float2x4(float4(10.0f, 20.0f, 30.0f, 40.0f), float4(10.0f, 20.0f, 30.0f, 40.0f));
  const float4 x_429 = float4(1.0f, 2.0f, 3.0f, 4.0f);
  const float4 x_430 = float4(2.0f, 4.0f, 6.0f, 8.0f);
  x_7_m = float2x4(x_429, x_430);
  x_439 = false;
  if (x_427) {
    x_438 = (all((x_429 == float4(1.0f, 2.0f, 3.0f, 4.0f))) & all((x_430 == float4(2.0f, 4.0f, 6.0f, 8.0f))));
    x_439 = x_438;
  }
  x_0_ok = x_439;
  x_8_m = float2x3(float3(7.0f, 9.0f, 11.0f), float3(8.0f, 10.0f, 12.0f));
  const float2x3 x_441 = float2x3(float3(39.0f, 49.0f, 59.0f), float3(54.0f, 68.0f, 82.0f));
  x_8_m = x_441;
  x_451 = false;
  if (x_439) {
    x_450 = (all((x_441[0u] == float3(39.0f, 49.0f, 59.0f))) & all((x_441[1u] == float3(54.0f, 68.0f, 82.0f))));
    x_451 = x_450;
  }
  x_0_ok = x_451;
  x_455 = false;
  if (x_451) {
    x_454 = test_matrix_op_matrix_half_b();
    x_455 = x_454;
  }
  if (x_455) {
    const float4 x_465 = asfloat(x_11[1]);
    x_456 = x_465;
  } else {
    const float4 x_468 = asfloat(x_11[0]);
    x_456 = x_468;
  }
  const float4 x_469 = x_456;
  return x_469;
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
