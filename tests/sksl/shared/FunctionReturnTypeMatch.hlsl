static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_31 : register(b0, space0) {
  uint4 x_31[2];
};

float2 returns_float2_f2() {
  return (2.0f).xx;
}

float3 returns_float3_f3() {
  return (3.0f).xxx;
}

float4 returns_float4_f4() {
  return (4.0f).xxxx;
}

float2x2 returns_float2x2_f22() {
  return float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
}

float3x3 returns_float3x3_f33() {
  return float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
}

float4x4 returns_float4x4_f44() {
  return float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
}

float returns_half_h() {
  return 1.0f;
}

float2 returns_half2_h2() {
  return (2.0f).xx;
}

float3 returns_half3_h3() {
  return (3.0f).xxx;
}

float4 returns_half4_h4() {
  return (4.0f).xxxx;
}

float2x2 returns_half2x2_h22() {
  return float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
}

float3x3 returns_half3x3_h33() {
  return float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
}

float4x4 returns_half4x4_h44() {
  return float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
}

bool returns_bool_b() {
  return true;
}

bool2 returns_bool2_b2() {
  return (true).xx;
}

bool3 returns_bool3_b3() {
  return (true).xxx;
}

bool4 returns_bool4_b4() {
  return (true).xxxx;
}

int returns_int_i() {
  return 1;
}

int2 returns_int2_i2() {
  return (2).xx;
}

int3 returns_int3_i3() {
  return (3).xxx;
}

int4 returns_int4_i4() {
  return (4).xxxx;
}

float4 main(inout float2 x_122) {
  float x1 = 0.0f;
  float2 x2 = float2(0.0f, 0.0f);
  float3 x3 = float3(0.0f, 0.0f, 0.0f);
  float4 x4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x5 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x6 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x7 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float x8 = 0.0f;
  float2 x9 = float2(0.0f, 0.0f);
  float3 x10 = float3(0.0f, 0.0f, 0.0f);
  float4 x11 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x12 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x13 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x14 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  bool x15 = false;
  bool2 x16 = bool2(false, false);
  bool3 x17 = bool3(false, false, false);
  bool4 x18 = bool4(false, false, false, false);
  int x19 = 0;
  int2 x20 = int2(0, 0);
  int3 x21 = int3(0, 0, 0);
  int4 x22 = int4(0, 0, 0, 0);
  float4 x_338 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_165 = false;
  bool x_166 = false;
  bool x_171 = false;
  bool x_172 = false;
  bool x_177 = false;
  bool x_178 = false;
  bool x_188 = false;
  bool x_189 = false;
  bool x_203 = false;
  bool x_204 = false;
  bool x_222 = false;
  bool x_223 = false;
  bool x_227 = false;
  bool x_228 = false;
  bool x_233 = false;
  bool x_234 = false;
  bool x_239 = false;
  bool x_240 = false;
  bool x_245 = false;
  bool x_246 = false;
  bool x_256 = false;
  bool x_257 = false;
  bool x_271 = false;
  bool x_272 = false;
  bool x_290 = false;
  bool x_291 = false;
  bool x_295 = false;
  bool x_296 = false;
  bool x_301 = false;
  bool x_302 = false;
  bool x_307 = false;
  bool x_308 = false;
  bool x_313 = false;
  bool x_314 = false;
  bool x_318 = false;
  bool x_319 = false;
  bool x_324 = false;
  bool x_325 = false;
  bool x_330 = false;
  bool x_331 = false;
  bool x_336 = false;
  bool x_337 = false;
  x1 = 1.0f;
  x2 = (2.0f).xx;
  x3 = (3.0f).xxx;
  x4 = (4.0f).xxxx;
  x5 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
  x6 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
  x7 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
  x8 = 1.0f;
  x9 = (2.0f).xx;
  x10 = (3.0f).xxx;
  x11 = (4.0f).xxxx;
  x12 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
  x13 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
  x14 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
  x15 = true;
  x16 = (true).xx;
  x17 = (true).xxx;
  x18 = (true).xxxx;
  x19 = 1;
  x20 = (2).xx;
  x21 = (3).xxx;
  x22 = (4).xxxx;
  x_166 = false;
  if (true) {
    const float2 x_163 = returns_float2_f2();
    x_165 = all(((2.0f).xx == x_163));
    x_166 = x_165;
  }
  x_172 = false;
  if (x_166) {
    const float3 x_169 = returns_float3_f3();
    x_171 = all(((3.0f).xxx == x_169));
    x_172 = x_171;
  }
  x_178 = false;
  if (x_172) {
    const float4 x_175 = returns_float4_f4();
    x_177 = all(((4.0f).xxxx == x_175));
    x_178 = x_177;
  }
  x_189 = false;
  if (x_178) {
    const float2x2 x_181 = returns_float2x2_f22();
    x_188 = (all((float2(2.0f, 0.0f) == x_181[0u])) & all((float2(0.0f, 2.0f) == x_181[1u])));
    x_189 = x_188;
  }
  x_204 = false;
  if (x_189) {
    const float3x3 x_192 = returns_float3x3_f33();
    x_203 = ((all((float3(3.0f, 0.0f, 0.0f) == x_192[0u])) & all((float3(0.0f, 3.0f, 0.0f) == x_192[1u]))) & all((float3(0.0f, 0.0f, 3.0f) == x_192[2u])));
    x_204 = x_203;
  }
  x_223 = false;
  if (x_204) {
    const float4x4 x_207 = returns_float4x4_f44();
    x_222 = (((all((float4(4.0f, 0.0f, 0.0f, 0.0f) == x_207[0u])) & all((float4(0.0f, 4.0f, 0.0f, 0.0f) == x_207[1u]))) & all((float4(0.0f, 0.0f, 4.0f, 0.0f) == x_207[2u]))) & all((float4(0.0f, 0.0f, 0.0f, 4.0f) == x_207[3u])));
    x_223 = x_222;
  }
  x_228 = false;
  if (x_223) {
    const float x_226 = returns_half_h();
    x_227 = (1.0f == x_226);
    x_228 = x_227;
  }
  x_234 = false;
  if (x_228) {
    const float2 x_231 = returns_half2_h2();
    x_233 = all(((2.0f).xx == x_231));
    x_234 = x_233;
  }
  x_240 = false;
  if (x_234) {
    const float3 x_237 = returns_half3_h3();
    x_239 = all(((3.0f).xxx == x_237));
    x_240 = x_239;
  }
  x_246 = false;
  if (x_240) {
    const float4 x_243 = returns_half4_h4();
    x_245 = all(((4.0f).xxxx == x_243));
    x_246 = x_245;
  }
  x_257 = false;
  if (x_246) {
    const float2x2 x_249 = returns_half2x2_h22();
    x_256 = (all((float2(2.0f, 0.0f) == x_249[0u])) & all((float2(0.0f, 2.0f) == x_249[1u])));
    x_257 = x_256;
  }
  x_272 = false;
  if (x_257) {
    const float3x3 x_260 = returns_half3x3_h33();
    x_271 = ((all((float3(3.0f, 0.0f, 0.0f) == x_260[0u])) & all((float3(0.0f, 3.0f, 0.0f) == x_260[1u]))) & all((float3(0.0f, 0.0f, 3.0f) == x_260[2u])));
    x_272 = x_271;
  }
  x_291 = false;
  if (x_272) {
    const float4x4 x_275 = returns_half4x4_h44();
    x_290 = (((all((float4(4.0f, 0.0f, 0.0f, 0.0f) == x_275[0u])) & all((float4(0.0f, 4.0f, 0.0f, 0.0f) == x_275[1u]))) & all((float4(0.0f, 0.0f, 4.0f, 0.0f) == x_275[2u]))) & all((float4(0.0f, 0.0f, 0.0f, 4.0f) == x_275[3u])));
    x_291 = x_290;
  }
  x_296 = false;
  if (x_291) {
    const bool x_294 = returns_bool_b();
    x_295 = (true == x_294);
    x_296 = x_295;
  }
  x_302 = false;
  if (x_296) {
    const bool2 x_299 = returns_bool2_b2();
    x_301 = all(((true).xx == x_299));
    x_302 = x_301;
  }
  x_308 = false;
  if (x_302) {
    const bool3 x_305 = returns_bool3_b3();
    x_307 = all(((true).xxx == x_305));
    x_308 = x_307;
  }
  x_314 = false;
  if (x_308) {
    const bool4 x_311 = returns_bool4_b4();
    x_313 = all(((true).xxxx == x_311));
    x_314 = x_313;
  }
  x_319 = false;
  if (x_314) {
    const int x_317 = returns_int_i();
    x_318 = (1 == x_317);
    x_319 = x_318;
  }
  x_325 = false;
  if (x_319) {
    const int2 x_322 = returns_int2_i2();
    x_324 = all(((2).xx == x_322));
    x_325 = x_324;
  }
  x_331 = false;
  if (x_325) {
    const int3 x_328 = returns_int3_i3();
    x_330 = all(((3).xxx == x_328));
    x_331 = x_330;
  }
  x_337 = false;
  if (x_331) {
    const int4 x_334 = returns_int4_i4();
    x_336 = all(((4).xxxx == x_334));
    x_337 = x_336;
  }
  if (x_337) {
    const float4 x_345 = asfloat(x_31[0]);
    x_338 = x_345;
  } else {
    const float4 x_347 = asfloat(x_31[1]);
    x_338 = x_347;
  }
  const float4 x_348 = x_338;
  return x_348;
}

void _entrypoint_1() {
  float2 x_41 = float2(0.0f, 0.0f);
  x_41 = (0.0f).xx;
  const float4 x_43 = main(x_41);
  sk_FragColor = x_43;
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
