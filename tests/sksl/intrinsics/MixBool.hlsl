static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

float4 main(inout float2 x_24) {
  int4 intGreen = int4(0, 0, 0, 0);
  int4 intRed = int4(0, 0, 0, 0);
  float4 x_410 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_76 = false;
  bool x_77 = false;
  bool x_90 = false;
  bool x_91 = false;
  bool x_98 = false;
  bool x_99 = false;
  bool x_104 = false;
  bool x_105 = false;
  bool x_116 = false;
  bool x_117 = false;
  bool x_128 = false;
  bool x_129 = false;
  bool x_135 = false;
  bool x_136 = false;
  bool x_139 = false;
  bool x_140 = false;
  bool x_147 = false;
  bool x_148 = false;
  bool x_154 = false;
  bool x_155 = false;
  bool x_160 = false;
  bool x_161 = false;
  bool x_164 = false;
  bool x_165 = false;
  bool x_171 = false;
  bool x_172 = false;
  bool x_178 = false;
  bool x_179 = false;
  bool x_184 = false;
  bool x_185 = false;
  bool x_204 = false;
  bool x_205 = false;
  bool x_225 = false;
  bool x_226 = false;
  bool x_247 = false;
  bool x_248 = false;
  bool x_263 = false;
  bool x_264 = false;
  bool x_283 = false;
  bool x_284 = false;
  bool x_304 = false;
  bool x_305 = false;
  bool x_325 = false;
  bool x_326 = false;
  bool x_341 = false;
  bool x_342 = false;
  bool x_348 = false;
  bool x_349 = false;
  bool x_358 = false;
  bool x_359 = false;
  bool x_367 = false;
  bool x_368 = false;
  bool x_375 = false;
  bool x_376 = false;
  bool x_382 = false;
  bool x_383 = false;
  bool x_391 = false;
  bool x_392 = false;
  bool x_400 = false;
  bool x_401 = false;
  bool x_408 = false;
  bool x_409 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const float4 x_35 = (x_33 * 100.0f);
  const int4 x_44 = int4(int(x_35.x), int(x_35.y), int(x_35.z), int(x_35.w));
  intGreen = x_44;
  const float4 x_48 = asfloat(x_10[1]);
  const float4 x_49 = (x_48 * 100.0f);
  const int4 x_58 = int4(int(x_49.x), int(x_49.y), int(x_49.z), int(x_49.w));
  intRed = x_58;
  const int x_61 = x_44.x;
  const int x_62 = x_58.x;
  x_77 = false;
  if (((false ? x_62 : x_61) == x_61)) {
    const int2 x_67 = int2(x_44.x, x_44.y);
    const int2 x_69 = int2(x_58.x, x_58.y);
    x_76 = all((((false).xx ? int2(x_58.x, x_58.y) : int2(x_44.x, x_44.y)) == int2(x_44.x, x_44.y)));
    x_77 = x_76;
  }
  x_91 = false;
  if (x_77) {
    const int3 x_81 = int3(x_44.x, x_44.y, x_44.z);
    const int3 x_83 = int3(x_58.x, x_58.y, x_58.z);
    x_90 = all((((false).xxx ? int3(x_58.x, x_58.y, x_58.z) : int3(x_44.x, x_44.y, x_44.z)) == int3(x_44.x, x_44.y, x_44.z)));
    x_91 = x_90;
  }
  x_99 = false;
  if (x_91) {
    x_98 = all((((false).xxxx ? x_58 : x_44) == x_44));
    x_99 = x_98;
  }
  x_105 = false;
  if (x_99) {
    x_104 = ((true ? x_62 : x_61) == x_62);
    x_105 = x_104;
  }
  x_117 = false;
  if (x_105) {
    const int2 x_109 = int2(x_44.x, x_44.y);
    const int2 x_110 = int2(x_58.x, x_58.y);
    x_116 = all((((true).xx ? int2(x_58.x, x_58.y) : int2(x_44.x, x_44.y)) == int2(x_58.x, x_58.y)));
    x_117 = x_116;
  }
  x_129 = false;
  if (x_117) {
    const int3 x_121 = int3(x_44.x, x_44.y, x_44.z);
    const int3 x_122 = int3(x_58.x, x_58.y, x_58.z);
    x_128 = all((((true).xxx ? int3(x_58.x, x_58.y, x_58.z) : int3(x_44.x, x_44.y, x_44.z)) == int3(x_58.x, x_58.y, x_58.z)));
    x_129 = x_128;
  }
  x_136 = false;
  if (x_129) {
    x_135 = all((((true).xxxx ? x_58 : x_44) == x_58));
    x_136 = x_135;
  }
  x_140 = false;
  if (x_136) {
    x_139 = (0 == x_61);
    x_140 = x_139;
  }
  x_148 = false;
  if (x_140) {
    x_147 = all((int2(0, 100) == int2(x_44.x, x_44.y)));
    x_148 = x_147;
  }
  x_155 = false;
  if (x_148) {
    x_154 = all((int3(0, 100, 0) == int3(x_44.x, x_44.y, x_44.z)));
    x_155 = x_154;
  }
  x_161 = false;
  if (x_155) {
    x_160 = all((int4(0, 100, 0, 100) == x_44));
    x_161 = x_160;
  }
  x_165 = false;
  if (x_161) {
    x_164 = (100 == x_62);
    x_165 = x_164;
  }
  x_172 = false;
  if (x_165) {
    x_171 = all((int2(100, 0) == int2(x_58.x, x_58.y)));
    x_172 = x_171;
  }
  x_179 = false;
  if (x_172) {
    x_178 = all((int3(100, 0, 0) == int3(x_58.x, x_58.y, x_58.z)));
    x_179 = x_178;
  }
  x_185 = false;
  if (x_179) {
    x_184 = all((int4(100, 0, 0, 100) == x_58));
    x_185 = x_184;
  }
  x_205 = false;
  if (x_185) {
    const float4 x_190 = asfloat(x_10[0]);
    const float x_191 = x_190.x;
    const float4 x_193 = asfloat(x_10[1]);
    const float x_194 = x_193.x;
    const float4 x_196 = asfloat(x_10[0]);
    const float4 x_199 = asfloat(x_10[1]);
    const float4 x_202 = asfloat(x_10[0]);
    x_204 = ((false ? x_199.x : x_196.x) == x_202.x);
    x_205 = x_204;
  }
  x_226 = false;
  if (x_205) {
    const float4 x_210 = asfloat(x_10[0]);
    const float2 x_211 = float2(x_210.x, x_210.y);
    const float4 x_213 = asfloat(x_10[1]);
    const float2 x_214 = float2(x_213.x, x_213.y);
    const float4 x_216 = asfloat(x_10[0]);
    const float4 x_219 = asfloat(x_10[1]);
    const float4 x_222 = asfloat(x_10[0]);
    x_225 = all((((false).xx ? float2(x_219.x, x_219.y) : float2(x_216.x, x_216.y)) == float2(x_222.x, x_222.y)));
    x_226 = x_225;
  }
  x_248 = false;
  if (x_226) {
    const float4 x_231 = asfloat(x_10[0]);
    const float3 x_232 = float3(x_231.x, x_231.y, x_231.z);
    const float4 x_235 = asfloat(x_10[1]);
    const float3 x_236 = float3(x_235.x, x_235.y, x_235.z);
    const float4 x_238 = asfloat(x_10[0]);
    const float4 x_241 = asfloat(x_10[1]);
    const float4 x_244 = asfloat(x_10[0]);
    x_247 = all((((false).xxx ? float3(x_241.x, x_241.y, x_241.z) : float3(x_238.x, x_238.y, x_238.z)) == float3(x_244.x, x_244.y, x_244.z)));
    x_248 = x_247;
  }
  x_264 = false;
  if (x_248) {
    const float4 x_253 = asfloat(x_10[0]);
    const float4 x_255 = asfloat(x_10[1]);
    const float4 x_257 = asfloat(x_10[0]);
    const float4 x_259 = asfloat(x_10[1]);
    const float4 x_261 = asfloat(x_10[0]);
    x_263 = all((((false).xxxx ? x_259 : x_257) == x_261));
    x_264 = x_263;
  }
  x_284 = false;
  if (x_264) {
    const float4 x_269 = asfloat(x_10[0]);
    const float x_270 = x_269.x;
    const float4 x_272 = asfloat(x_10[1]);
    const float x_273 = x_272.x;
    const float4 x_275 = asfloat(x_10[0]);
    const float4 x_278 = asfloat(x_10[1]);
    const float4 x_281 = asfloat(x_10[1]);
    x_283 = ((true ? x_278.x : x_275.x) == x_281.x);
    x_284 = x_283;
  }
  x_305 = false;
  if (x_284) {
    const float4 x_289 = asfloat(x_10[0]);
    const float2 x_290 = float2(x_289.x, x_289.y);
    const float4 x_292 = asfloat(x_10[1]);
    const float2 x_293 = float2(x_292.x, x_292.y);
    const float4 x_295 = asfloat(x_10[0]);
    const float4 x_298 = asfloat(x_10[1]);
    const float4 x_301 = asfloat(x_10[1]);
    x_304 = all((((true).xx ? float2(x_298.x, x_298.y) : float2(x_295.x, x_295.y)) == float2(x_301.x, x_301.y)));
    x_305 = x_304;
  }
  x_326 = false;
  if (x_305) {
    const float4 x_310 = asfloat(x_10[0]);
    const float3 x_311 = float3(x_310.x, x_310.y, x_310.z);
    const float4 x_313 = asfloat(x_10[1]);
    const float3 x_314 = float3(x_313.x, x_313.y, x_313.z);
    const float4 x_316 = asfloat(x_10[0]);
    const float4 x_319 = asfloat(x_10[1]);
    const float4 x_322 = asfloat(x_10[1]);
    x_325 = all((((true).xxx ? float3(x_319.x, x_319.y, x_319.z) : float3(x_316.x, x_316.y, x_316.z)) == float3(x_322.x, x_322.y, x_322.z)));
    x_326 = x_325;
  }
  x_342 = false;
  if (x_326) {
    const float4 x_331 = asfloat(x_10[0]);
    const float4 x_333 = asfloat(x_10[1]);
    const float4 x_335 = asfloat(x_10[0]);
    const float4 x_337 = asfloat(x_10[1]);
    const float4 x_339 = asfloat(x_10[1]);
    x_341 = all((((true).xxxx ? x_337 : x_335) == x_339));
    x_342 = x_341;
  }
  x_349 = false;
  if (x_342) {
    const float4 x_346 = asfloat(x_10[0]);
    x_348 = (0.0f == x_346.x);
    x_349 = x_348;
  }
  x_359 = false;
  if (x_349) {
    const float4 x_355 = asfloat(x_10[0]);
    x_358 = all((float2(0.0f, 1.0f) == float2(x_355.x, x_355.y)));
    x_359 = x_358;
  }
  x_368 = false;
  if (x_359) {
    const float4 x_364 = asfloat(x_10[0]);
    x_367 = all((float3(0.0f, 1.0f, 0.0f) == float3(x_364.x, x_364.y, x_364.z)));
    x_368 = x_367;
  }
  x_376 = false;
  if (x_368) {
    const float4 x_373 = asfloat(x_10[0]);
    x_375 = all((float4(0.0f, 1.0f, 0.0f, 1.0f) == x_373));
    x_376 = x_375;
  }
  x_383 = false;
  if (x_376) {
    const float4 x_380 = asfloat(x_10[1]);
    x_382 = (1.0f == x_380.x);
    x_383 = x_382;
  }
  x_392 = false;
  if (x_383) {
    const float4 x_388 = asfloat(x_10[1]);
    x_391 = all((float2(1.0f, 0.0f) == float2(x_388.x, x_388.y)));
    x_392 = x_391;
  }
  x_401 = false;
  if (x_392) {
    const float4 x_397 = asfloat(x_10[1]);
    x_400 = all((float3(1.0f, 0.0f, 0.0f) == float3(x_397.x, x_397.y, x_397.z)));
    x_401 = x_400;
  }
  x_409 = false;
  if (x_401) {
    const float4 x_406 = asfloat(x_10[1]);
    x_408 = all((float4(1.0f, 0.0f, 0.0f, 1.0f) == x_406));
    x_409 = x_408;
  }
  if (x_409) {
    const float4 x_416 = asfloat(x_10[0]);
    x_410 = x_416;
  } else {
    const float4 x_418 = asfloat(x_10[1]);
    x_410 = x_418;
  }
  const float4 x_419 = x_410;
  return x_419;
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
