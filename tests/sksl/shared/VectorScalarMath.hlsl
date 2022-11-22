static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[3];
};

int2 tint_div(int2 lhs, int2 rhs) {
  return (lhs / (((rhs == (0).xx) | ((lhs == (-2147483648).xx) & (rhs == (-1).xx))) ? (1).xx : rhs));
}

int4 tint_div_1(int4 lhs, int4 rhs) {
  return (lhs / (((rhs == (0).xxxx) | ((lhs == (-2147483648).xxxx) & (rhs == (-1).xxxx))) ? (1).xxxx : rhs));
}

bool test_int_b() {
  bool ok = false;
  int4 inputRed = int4(0, 0, 0, 0);
  int4 inputGreen = int4(0, 0, 0, 0);
  int4 x = int4(0, 0, 0, 0);
  bool x_70 = false;
  bool x_71 = false;
  bool x_80 = false;
  bool x_81 = false;
  bool x_89 = false;
  bool x_90 = false;
  bool x_102 = false;
  bool x_103 = false;
  bool x_115 = false;
  bool x_116 = false;
  bool x_125 = false;
  bool x_126 = false;
  bool x_131 = false;
  bool x_132 = false;
  bool x_141 = false;
  bool x_142 = false;
  bool x_150 = false;
  bool x_151 = false;
  bool x_162 = false;
  bool x_163 = false;
  bool x_175 = false;
  bool x_176 = false;
  bool x_185 = false;
  bool x_186 = false;
  bool x_195 = false;
  bool x_196 = false;
  bool x_204 = false;
  bool x_205 = false;
  ok = true;
  const float4 x_36 = asfloat(x_11[0]);
  const int4 x_45 = int4(int(x_36.x), int(x_36.y), int(x_36.z), int(x_36.w));
  inputRed = x_45;
  const float4 x_49 = asfloat(x_11[1]);
  const int4 x_58 = int4(int(x_49.x), int(x_49.y), int(x_49.z), int(x_49.w));
  inputGreen = x_58;
  const int4 x_62 = (x_45 + (2).xxxx);
  x = x_62;
  x_71 = false;
  if (true) {
    x_70 = all((x_62 == int4(3, 2, 2, 3)));
    x_71 = x_70;
  }
  ok = x_71;
  const int4 x_73 = (int4(x_58.y, x_58.w, x_58.x, x_58.z) - (2).xxxx);
  x = x_73;
  x_81 = false;
  if (x_71) {
    x_80 = all((x_73 == int4(-1, -1, -2, -2)));
    x_81 = x_80;
  }
  ok = x_81;
  const int x_82 = x_58.y;
  const int4 x_84 = (x_45 + int4(x_82, x_82, x_82, x_82));
  x = x_84;
  x_90 = false;
  if (x_81) {
    x_89 = all((x_84 == int4(2, 1, 1, 2)));
    x_90 = x_89;
  }
  ok = x_90;
  const int3 x_95 = (int3(x_58.w, x_58.y, x_58.w) * (9).xxx);
  const int4 x_96 = x;
  const int4 x_97 = int4(x_95.x, x_95.y, x_95.z, x_96.w);
  x = x_97;
  x_103 = false;
  if (x_90) {
    x_102 = all((x_97 == int4(9, 9, 9, 2)));
    x_103 = x_102;
  }
  ok = x_103;
  const int2 x_108 = tint_div(int2(x_97.z, x_97.w), (4).xx);
  const int4 x_109 = x;
  const int4 x_110 = int4(x_108.x, x_108.y, x_109.z, x_109.w);
  x = x_110;
  x_116 = false;
  if (x_103) {
    x_115 = all((x_110 == int4(2, 0, 9, 2)));
    x_116 = x_115;
  }
  ok = x_116;
  const int4 x_119 = (x_45 * (5).xxxx);
  const int4 x_120 = int4(x_119.y, x_119.x, x_119.w, x_119.z);
  x = x_120;
  x_126 = false;
  if (x_116) {
    x_125 = all((x_120 == int4(0, 5, 5, 0)));
    x_126 = x_125;
  }
  ok = x_126;
  const int4 x_127 = ((2).xxxx + x_45);
  x = x_127;
  x_132 = false;
  if (x_126) {
    x_131 = all((x_127 == int4(3, 2, 2, 3)));
    x_132 = x_131;
  }
  ok = x_132;
  const int4 x_136 = ((10).xxxx - int4(x_58.y, x_58.w, x_58.x, x_58.z));
  x = x_136;
  x_142 = false;
  if (x_132) {
    x_141 = all((x_136 == int4(9, 9, 10, 10)));
    x_142 = x_141;
  }
  ok = x_142;
  const int x_143 = x_45.x;
  const int4 x_145 = (int4(x_143, x_143, x_143, x_143) + x_58);
  x = x_145;
  x_151 = false;
  if (x_142) {
    x_150 = all((x_145 == int4(1, 2, 1, 2)));
    x_151 = x_150;
  }
  ok = x_151;
  const int3 x_155 = ((8).xxx * int3(x_58.w, x_58.y, x_58.w));
  const int4 x_156 = x;
  const int4 x_157 = int4(x_155.x, x_155.y, x_155.z, x_156.w);
  x = x_157;
  x_163 = false;
  if (x_151) {
    x_162 = all((x_157 == int4(8, 8, 8, 2)));
    x_163 = x_162;
  }
  ok = x_163;
  const int2 x_167 = tint_div((36).xx, int2(x_157.z, x_157.w));
  const int4 x_168 = x;
  const int4 x_169 = int4(x_167.x, x_167.y, x_168.z, x_168.w);
  x = x_169;
  x_176 = false;
  if (x_163) {
    x_175 = all((x_169 == int4(4, 18, 8, 2)));
    x_176 = x_175;
  }
  ok = x_176;
  const int4 x_179 = tint_div_1((37).xxxx, x_169);
  const int4 x_180 = int4(x_179.y, x_179.x, x_179.w, x_179.z);
  x = x_180;
  x_186 = false;
  if (x_176) {
    x_185 = all((x_180 == int4(2, 9, 18, 4)));
    x_186 = x_185;
  }
  ok = x_186;
  const int4 x_187 = (x_180 + (2).xxxx);
  x = x_187;
  const int4 x_188 = (x_187 * (2).xxxx);
  x = x_188;
  const int4 x_190 = (x_188 - (4).xxxx);
  x = x_190;
  const int4 x_191 = tint_div_1(x_190, (2).xxxx);
  x = x_191;
  x_196 = false;
  if (x_186) {
    x_195 = all((x_191 == int4(2, 9, 18, 4)));
    x_196 = x_195;
  }
  ok = x_196;
  const int4 x_197 = (x_191 + (2).xxxx);
  x = x_197;
  const int4 x_198 = (x_197 * (2).xxxx);
  x = x_198;
  const int4 x_199 = (x_198 - (4).xxxx);
  x = x_199;
  const int4 x_200 = tint_div_1(x_199, (2).xxxx);
  x = x_200;
  x_205 = false;
  if (x_196) {
    x_204 = all((x_200 == int4(2, 9, 18, 4)));
    x_205 = x_204;
  }
  ok = x_205;
  return x_205;
}

float4 main(inout float2 x_207) {
  bool x_0_ok = false;
  float4 x_1_inputRed = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_2_inputGreen = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_3_x = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_364 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_226 = false;
  bool x_227 = false;
  bool x_236 = false;
  bool x_237 = false;
  bool x_246 = false;
  bool x_247 = false;
  bool x_258 = false;
  bool x_259 = false;
  bool x_270 = false;
  bool x_271 = false;
  bool x_279 = false;
  bool x_280 = false;
  bool x_285 = false;
  bool x_286 = false;
  bool x_295 = false;
  bool x_296 = false;
  bool x_304 = false;
  bool x_305 = false;
  bool x_315 = false;
  bool x_316 = false;
  bool x_328 = false;
  bool x_329 = false;
  bool x_337 = false;
  bool x_338 = false;
  bool x_348 = false;
  bool x_349 = false;
  bool x_358 = false;
  bool x_359 = false;
  bool x_362 = false;
  bool x_363 = false;
  x_0_ok = true;
  const float4 x_213 = asfloat(x_11[0]);
  x_1_inputRed = x_213;
  const float4 x_216 = asfloat(x_11[1]);
  x_2_inputGreen = x_216;
  const float4 x_220 = (x_213 + (2.0f).xxxx);
  x_3_x = x_220;
  x_227 = false;
  if (true) {
    x_226 = all((x_220 == float4(3.0f, 2.0f, 2.0f, 3.0f)));
    x_227 = x_226;
  }
  x_0_ok = x_227;
  const float4 x_229 = (float4(x_216.y, x_216.w, x_216.x, x_216.z) - (2.0f).xxxx);
  x_3_x = x_229;
  x_237 = false;
  if (x_227) {
    x_236 = all((x_229 == float4(-1.0f, -1.0f, -2.0f, -2.0f)));
    x_237 = x_236;
  }
  x_0_ok = x_237;
  const float x_238 = x_216.y;
  const float4 x_240 = (x_213 + float4(x_238, x_238, x_238, x_238));
  x_3_x = x_240;
  x_247 = false;
  if (x_237) {
    x_246 = all((x_240 == float4(2.0f, 1.0f, 1.0f, 2.0f)));
    x_247 = x_246;
  }
  x_0_ok = x_247;
  const float3 x_251 = (float3(x_216.w, x_216.y, x_216.w) * 9.0f);
  const float4 x_252 = x_3_x;
  const float4 x_253 = float4(x_251.x, x_251.y, x_251.z, x_252.w);
  x_3_x = x_253;
  x_259 = false;
  if (x_247) {
    x_258 = all((x_253 == float4(9.0f, 9.0f, 9.0f, 2.0f)));
    x_259 = x_258;
  }
  x_0_ok = x_259;
  const float2 x_261 = (float2(x_253.z, x_253.w) * 2.0f);
  const float4 x_262 = x_3_x;
  const float4 x_263 = float4(x_261.x, x_261.y, x_262.z, x_262.w);
  x_3_x = x_263;
  x_271 = false;
  if (x_259) {
    x_270 = all((x_263 == float4(18.0f, 4.0f, 9.0f, 2.0f)));
    x_271 = x_270;
  }
  x_0_ok = x_271;
  const float4 x_273 = (x_213 * 5.0f);
  const float4 x_274 = float4(x_273.y, x_273.x, x_273.w, x_273.z);
  x_3_x = x_274;
  x_280 = false;
  if (x_271) {
    x_279 = all((x_274 == float4(0.0f, 5.0f, 5.0f, 0.0f)));
    x_280 = x_279;
  }
  x_0_ok = x_280;
  const float4 x_281 = ((2.0f).xxxx + x_213);
  x_3_x = x_281;
  x_286 = false;
  if (x_280) {
    x_285 = all((x_281 == float4(3.0f, 2.0f, 2.0f, 3.0f)));
    x_286 = x_285;
  }
  x_0_ok = x_286;
  const float4 x_290 = ((10.0f).xxxx - float4(x_216.y, x_216.w, x_216.x, x_216.z));
  x_3_x = x_290;
  x_296 = false;
  if (x_286) {
    x_295 = all((x_290 == float4(9.0f, 9.0f, 10.0f, 10.0f)));
    x_296 = x_295;
  }
  x_0_ok = x_296;
  const float x_297 = x_213.x;
  const float4 x_299 = (float4(x_297, x_297, x_297, x_297) + x_216);
  x_3_x = x_299;
  x_305 = false;
  if (x_296) {
    x_304 = all((x_299 == float4(1.0f, 2.0f, 1.0f, 2.0f)));
    x_305 = x_304;
  }
  x_0_ok = x_305;
  const float3 x_308 = (float3(x_216.w, x_216.y, x_216.w) * 8.0f);
  const float4 x_309 = x_3_x;
  const float4 x_310 = float4(x_308.x, x_308.y, x_308.z, x_309.w);
  x_3_x = x_310;
  x_316 = false;
  if (x_305) {
    x_315 = all((x_310 == float4(8.0f, 8.0f, 8.0f, 2.0f)));
    x_316 = x_315;
  }
  x_0_ok = x_316;
  const float2 x_320 = ((32.0f).xx / float2(x_310.z, x_310.w));
  const float4 x_321 = x_3_x;
  const float4 x_322 = float4(x_320.x, x_320.y, x_321.z, x_321.w);
  x_3_x = x_322;
  x_329 = false;
  if (x_316) {
    x_328 = all((x_322 == float4(4.0f, 16.0f, 8.0f, 2.0f)));
    x_329 = x_328;
  }
  x_0_ok = x_329;
  const float4 x_331 = ((32.0f).xxxx / x_322);
  const float4 x_332 = float4(x_331.y, x_331.x, x_331.w, x_331.z);
  x_3_x = x_332;
  x_338 = false;
  if (x_329) {
    x_337 = all((x_332 == float4(2.0f, 8.0f, 16.0f, 4.0f)));
    x_338 = x_337;
  }
  x_0_ok = x_338;
  const float4 x_339 = (x_332 + (2.0f).xxxx);
  x_3_x = x_339;
  const float4 x_340 = (x_339 * 2.0f);
  x_3_x = x_340;
  const float4 x_342 = (x_340 - (4.0f).xxxx);
  x_3_x = x_342;
  const float4 x_344 = (x_342 * 0.5f);
  x_3_x = x_344;
  x_349 = false;
  if (x_338) {
    x_348 = all((x_344 == float4(2.0f, 8.0f, 16.0f, 4.0f)));
    x_349 = x_348;
  }
  x_0_ok = x_349;
  const float4 x_350 = (x_344 + (2.0f).xxxx);
  x_3_x = x_350;
  const float4 x_351 = (x_350 * 2.0f);
  x_3_x = x_351;
  const float4 x_352 = (x_351 - (4.0f).xxxx);
  x_3_x = x_352;
  const float4 x_354 = (x_352 * 0.5f);
  x_3_x = x_354;
  x_359 = false;
  if (x_349) {
    x_358 = all((x_354 == float4(2.0f, 8.0f, 16.0f, 4.0f)));
    x_359 = x_358;
  }
  x_0_ok = x_359;
  x_363 = false;
  if (x_359) {
    x_362 = test_int_b();
    x_363 = x_362;
  }
  if (x_363) {
    const float4 x_369 = asfloat(x_11[1]);
    x_364 = x_369;
  } else {
    const float4 x_371 = asfloat(x_11[0]);
    x_364 = x_371;
  }
  const float4 x_372 = x_364;
  return x_372;
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
