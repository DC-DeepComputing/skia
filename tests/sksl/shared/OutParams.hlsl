static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_32 : register(b0, space0) {
  uint4 x_32[3];
};

void out_half_vh(inout float x_47) {
  const float4 x_53 = asfloat(x_32[2]);
  x_47 = x_53.x;
  return;
}

void out_half2_vh2(inout float2 x_56) {
  const float4 x_59 = asfloat(x_32[2]);
  const float x_60 = x_59.y;
  x_56 = float2(x_60, x_60);
  return;
}

void out_half3_vh3(inout float3 x_65) {
  const float4 x_68 = asfloat(x_32[2]);
  const float x_69 = x_68.z;
  x_65 = float3(x_69, x_69, x_69);
  return;
}

void out_half4_vh4(inout float4 x_73) {
  const float4 x_76 = asfloat(x_32[2]);
  const float x_77 = x_76.w;
  x_73 = float4(x_77, x_77, x_77, x_77);
  return;
}

void out_half2x2_vh22(inout float2x2 x_82) {
  const float4 x_85 = asfloat(x_32[2]);
  const float x_86 = x_85.x;
  x_82 = float2x2(float2(x_86, 0.0f), float2(0.0f, x_86));
  return;
}

void out_half3x3_vh33(inout float3x3 x_93) {
  const float4 x_96 = asfloat(x_32[2]);
  const float x_97 = x_96.y;
  x_93 = float3x3(float3(x_97, 0.0f, 0.0f), float3(0.0f, x_97, 0.0f), float3(0.0f, 0.0f, x_97));
  return;
}

void out_half4x4_vh44(inout float4x4 x_105) {
  const float4 x_108 = asfloat(x_32[2]);
  const float x_109 = x_108.z;
  x_105 = float4x4(float4(x_109, 0.0f, 0.0f, 0.0f), float4(0.0f, x_109, 0.0f, 0.0f), float4(0.0f, 0.0f, x_109, 0.0f), float4(0.0f, 0.0f, 0.0f, x_109));
  return;
}

void out_int_vi(inout int x_117) {
  const float4 x_120 = asfloat(x_32[2]);
  x_117 = int(x_120.x);
  return;
}

void out_int2_vi2(inout int2 x_126) {
  const float4 x_129 = asfloat(x_32[2]);
  const int x_131 = int(x_129.y);
  x_126 = int2(x_131, x_131);
  return;
}

void out_int3_vi3(inout int3 x_136) {
  const float4 x_139 = asfloat(x_32[2]);
  const int x_141 = int(x_139.z);
  x_136 = int3(x_141, x_141, x_141);
  return;
}

void out_int4_vi4(inout int4 x_146) {
  const float4 x_149 = asfloat(x_32[2]);
  const int x_151 = int(x_149.w);
  x_146 = int4(x_151, x_151, x_151, x_151);
  return;
}

void out_float_vf(inout float x_153) {
  const float4 x_156 = asfloat(x_32[2]);
  x_153 = x_156.x;
  return;
}

void out_float2_vf2(inout float2 x_158) {
  const float4 x_161 = asfloat(x_32[2]);
  const float x_162 = x_161.y;
  x_158 = float2(x_162, x_162);
  return;
}

void out_float3_vf3(inout float3 x_164) {
  const float4 x_167 = asfloat(x_32[2]);
  const float x_168 = x_167.z;
  x_164 = float3(x_168, x_168, x_168);
  return;
}

void out_float4_vf4(inout float4 x_170) {
  const float4 x_173 = asfloat(x_32[2]);
  const float x_174 = x_173.w;
  x_170 = float4(x_174, x_174, x_174, x_174);
  return;
}

void out_float2x2_vf22(inout float2x2 x_176) {
  const float4 x_179 = asfloat(x_32[2]);
  const float x_180 = x_179.x;
  x_176 = float2x2(float2(x_180, 0.0f), float2(0.0f, x_180));
  return;
}

void out_float3x3_vf33(inout float3x3 x_184) {
  const float4 x_187 = asfloat(x_32[2]);
  const float x_188 = x_187.y;
  x_184 = float3x3(float3(x_188, 0.0f, 0.0f), float3(0.0f, x_188, 0.0f), float3(0.0f, 0.0f, x_188));
  return;
}

void out_float4x4_vf44(inout float4x4 x_193) {
  const float4 x_196 = asfloat(x_32[2]);
  const float x_197 = x_196.z;
  x_193 = float4x4(float4(x_197, 0.0f, 0.0f, 0.0f), float4(0.0f, x_197, 0.0f, 0.0f), float4(0.0f, 0.0f, x_197, 0.0f), float4(0.0f, 0.0f, 0.0f, x_197));
  return;
}

void out_bool_vb(inout bool x_205) {
  const float4 x_208 = asfloat(x_32[2]);
  x_205 = !((x_208.x == 0.0f));
  return;
}

void out_bool2_vb2(inout bool2 x_214) {
  const float4 x_217 = asfloat(x_32[2]);
  const bool x_219 = !((x_217.y == 0.0f));
  x_214 = bool2(x_219, x_219);
  return;
}

void out_bool3_vb3(inout bool3 x_224) {
  const float4 x_227 = asfloat(x_32[2]);
  const bool x_229 = !((x_227.z == 0.0f));
  x_224 = bool3(x_229, x_229, x_229);
  return;
}

void out_bool4_vb4(inout bool4 x_234) {
  const float4 x_237 = asfloat(x_32[2]);
  const bool x_239 = !((x_237.w == 0.0f));
  x_234 = bool4(x_239, x_239, x_239, x_239);
  return;
}

float4 main(inout float2 x_242) {
  float h = 0.0f;
  float x_245 = 0.0f;
  float2 h2 = float2(0.0f, 0.0f);
  float2 x_249 = float2(0.0f, 0.0f);
  float3 h3 = float3(0.0f, 0.0f, 0.0f);
  float3 x_253 = float3(0.0f, 0.0f, 0.0f);
  float4 h4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_257 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_262 = 0.0f;
  float2 x_265 = float2(0.0f, 0.0f);
  float4 x_270 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 h2x2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_276 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 h3x3 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x_280 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 h4x4 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_284 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 x_288 = float3(0.0f, 0.0f, 0.0f);
  float x_294 = 0.0f;
  float x_300 = 0.0f;
  int i = 0;
  int x_304 = 0;
  int2 i2 = int2(0, 0);
  int2 x_308 = int2(0, 0);
  int3 i3 = int3(0, 0, 0);
  int3 x_312 = int3(0, 0, 0);
  int4 i4 = int4(0, 0, 0, 0);
  int4 x_316 = int4(0, 0, 0, 0);
  int3 x_319 = int3(0, 0, 0);
  int x_325 = 0;
  float f = 0.0f;
  float x_329 = 0.0f;
  float2 f2 = float2(0.0f, 0.0f);
  float2 x_333 = float2(0.0f, 0.0f);
  float3 f3 = float3(0.0f, 0.0f, 0.0f);
  float3 x_337 = float3(0.0f, 0.0f, 0.0f);
  float4 f4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_341 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 x_344 = float2(0.0f, 0.0f);
  float x_350 = 0.0f;
  float2x2 f2x2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_354 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 f3x3 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x_358 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 f4x4 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_362 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float x_367 = 0.0f;
  bool b = false;
  bool x_371 = false;
  bool2 b2 = bool2(false, false);
  bool2 x_375 = bool2(false, false);
  bool3 b3 = bool3(false, false, false);
  bool3 x_379 = bool3(false, false, false);
  bool4 b4 = bool4(false, false, false, false);
  bool4 x_383 = bool4(false, false, false, false);
  bool2 x_386 = bool2(false, false);
  bool x_392 = false;
  bool ok = false;
  float4 x_484 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_423 = false;
  bool x_424 = false;
  bool x_449 = false;
  bool x_450 = false;
  bool x_463 = false;
  bool x_464 = false;
  bool x_482 = false;
  bool x_483 = false;
  out_half_vh(x_245);
  const float x_247 = x_245;
  h = x_247;
  out_half2_vh2(x_249);
  const float2 x_251 = x_249;
  h2 = x_251;
  out_half3_vh3(x_253);
  const float3 x_255 = x_253;
  h3 = x_255;
  out_half4_vh4(x_257);
  const float4 x_259 = x_257;
  h4 = x_259;
  out_half_vh(x_262);
  const float x_264 = x_262;
  h3.y = x_264;
  out_half2_vh2(x_265);
  const float2 x_267 = x_265;
  const float3 x_268 = h3;
  h3 = float3(x_267.x, x_268.y, x_267.y);
  out_half4_vh4(x_270);
  const float4 x_272 = x_270;
  const float4 x_273 = h4;
  h4 = float4(x_272.z, x_272.w, x_272.x, x_272.y);
  out_half2x2_vh22(x_276);
  const float2x2 x_278 = x_276;
  h2x2 = x_278;
  out_half3x3_vh33(x_280);
  const float3x3 x_282 = x_280;
  h3x3 = x_282;
  out_half4x4_vh44(x_284);
  const float4x4 x_286 = x_284;
  h4x4 = x_286;
  out_half3_vh3(x_288);
  const float3 x_290 = x_288;
  h3x3[1] = x_290;
  out_half_vh(x_294);
  const float x_296 = x_294;
  h4x4[3].w = x_296;
  out_half_vh(x_300);
  const float x_302 = x_300;
  h2x2[0].x = x_302;
  out_int_vi(x_304);
  const int x_306 = x_304;
  i = x_306;
  out_int2_vi2(x_308);
  const int2 x_310 = x_308;
  i2 = x_310;
  out_int3_vi3(x_312);
  const int3 x_314 = x_312;
  i3 = x_314;
  out_int4_vi4(x_316);
  const int4 x_318 = x_316;
  i4 = x_318;
  out_int3_vi3(x_319);
  const int3 x_321 = x_319;
  const int4 x_322 = i4;
  i4 = int4(x_321.x, x_321.y, x_321.z, x_322.w);
  out_int_vi(x_325);
  const int x_327 = x_325;
  i2.y = x_327;
  out_float_vf(x_329);
  const float x_331 = x_329;
  f = x_331;
  out_float2_vf2(x_333);
  const float2 x_335 = x_333;
  f2 = x_335;
  out_float3_vf3(x_337);
  const float3 x_339 = x_337;
  f3 = x_339;
  out_float4_vf4(x_341);
  const float4 x_343 = x_341;
  f4 = x_343;
  out_float2_vf2(x_344);
  const float2 x_346 = x_344;
  const float3 x_347 = f3;
  f3 = float3(x_346.x, x_346.y, x_347.z);
  out_float_vf(x_350);
  const float x_352 = x_350;
  f2.x = x_352;
  out_float2x2_vf22(x_354);
  const float2x2 x_356 = x_354;
  f2x2 = x_356;
  out_float3x3_vf33(x_358);
  const float3x3 x_360 = x_358;
  f3x3 = x_360;
  out_float4x4_vf44(x_362);
  const float4x4 x_364 = x_362;
  f4x4 = x_364;
  out_float_vf(x_367);
  const float x_369 = x_367;
  f2x2[0].x = x_369;
  out_bool_vb(x_371);
  const bool x_373 = x_371;
  b = x_373;
  out_bool2_vb2(x_375);
  const bool2 x_377 = x_375;
  b2 = x_377;
  out_bool3_vb3(x_379);
  const bool3 x_381 = x_379;
  b3 = x_381;
  out_bool4_vb4(x_383);
  const bool4 x_385 = x_383;
  b4 = x_385;
  out_bool2_vb2(x_386);
  const bool2 x_388 = x_386;
  const bool4 x_389 = b4;
  b4 = bool4(x_388.x, x_389.y, x_389.z, x_388.y);
  out_bool_vb(x_392);
  const bool x_394 = x_392;
  b3.z = x_394;
  ok = true;
  x_424 = false;
  if (true) {
    const float x_401 = h;
    const float2 x_402 = h2;
    const float3 x_405 = h3;
    const float4 x_408 = h4;
    const float2 x_412 = h2x2[0];
    const float3 x_416 = h3x3[0];
    const float4 x_420 = h4x4[0];
    x_423 = (1.0f == ((((((x_401 * x_402.x) * x_405.x) * x_408.x) * x_412.x) * x_416.x) * x_420.x));
    x_424 = x_423;
  }
  ok = x_424;
  x_450 = false;
  if (x_424) {
    const float x_427 = f;
    const float2 x_428 = f2;
    const float3 x_431 = f3;
    const float4 x_434 = f4;
    const float2 x_438 = f2x2[0];
    const float3 x_442 = f3x3[0];
    const float4 x_446 = f4x4[0];
    x_449 = (1.0f == ((((((x_427 * x_428.x) * x_431.x) * x_434.x) * x_438.x) * x_442.x) * x_446.x));
    x_450 = x_449;
  }
  ok = x_450;
  x_464 = false;
  if (x_450) {
    const int x_453 = i;
    const int2 x_454 = i2;
    const int3 x_457 = i3;
    const int4 x_460 = i4;
    x_463 = (1 == (((x_453 * x_454.x) * x_457.x) * x_460.x));
    x_464 = x_463;
  }
  bool x_471 = false;
  bool x_472 = false;
  bool x_476 = false;
  bool x_477 = false;
  bool x_481 = false;
  ok = x_464;
  x_483 = false;
  if (x_464) {
    const bool x_467 = b;
    x_472 = false;
    if (x_467) {
      const bool2 x_470 = b2;
      x_471 = x_470.x;
      x_472 = x_471;
    }
    x_477 = false;
    if (x_472) {
      const bool3 x_475 = b3;
      x_476 = x_475.x;
      x_477 = x_476;
    }
    x_482 = false;
    if (x_477) {
      const bool4 x_480 = b4;
      x_481 = x_480.x;
      x_482 = x_481;
    }
    x_483 = x_482;
  }
  ok = x_483;
  if (x_483) {
    const float4 x_489 = asfloat(x_32[0]);
    x_484 = x_489;
  } else {
    const float4 x_491 = asfloat(x_32[1]);
    x_484 = x_491;
  }
  const float4 x_492 = x_484;
  return x_492;
}

void _entrypoint_1() {
  float2 x_42 = float2(0.0f, 0.0f);
  x_42 = (0.0f).xx;
  const float4 x_44 = main(x_42);
  sk_FragColor = x_44;
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
