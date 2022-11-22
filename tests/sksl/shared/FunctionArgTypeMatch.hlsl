static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_33 : register(b0, space0) {
  uint4 x_33[2];
};

bool takes_void_b() {
  return true;
}

bool takes_float_bf(inout float x_51) {
  return true;
}

bool takes_float2_bf2(inout float2 x_54) {
  return true;
}

bool takes_float3_bf3(inout float3 x_59) {
  return true;
}

bool takes_float4_bf4(inout float4 x_63) {
  return true;
}

bool takes_float2x2_bf22(inout float2x2 x_68) {
  return true;
}

bool takes_float3x3_bf33(inout float3x3 x_73) {
  return true;
}

bool takes_float4x4_bf44(inout float4x4 x_78) {
  return true;
}

bool takes_half_bh(inout float x_80) {
  return true;
}

bool takes_half2_bh2(inout float2 x_82) {
  return true;
}

bool takes_half3_bh3(inout float3 x_84) {
  return true;
}

bool takes_half4_bh4(inout float4 x_86) {
  return true;
}

bool takes_half2x2_bh22(inout float2x2 x_88) {
  return true;
}

bool takes_half3x3_bh33(inout float3x3 x_90) {
  return true;
}

bool takes_half4x4_bh44(inout float4x4 x_92) {
  return true;
}

bool takes_bool_bb(inout bool x_96) {
  return true;
}

bool takes_bool2_bb2(inout bool2 x_101) {
  return true;
}

bool takes_bool3_bb3(inout bool3 x_106) {
  return true;
}

bool takes_bool4_bb4(inout bool4 x_111) {
  return true;
}

bool takes_int_bi(inout int x_116) {
  return true;
}

bool takes_int2_bi2(inout int2 x_121) {
  return true;
}

bool takes_int3_bi3(inout int3 x_126) {
  return true;
}

bool takes_int4_bi4(inout int4 x_131) {
  return true;
}

float4 main(inout float2 x_134) {
  float x_144 = 0.0f;
  float2 x_151 = float2(0.0f, 0.0f);
  float3 x_158 = float3(0.0f, 0.0f, 0.0f);
  float4 x_165 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_173 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x_182 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_192 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float x_197 = 0.0f;
  float2 x_202 = float2(0.0f, 0.0f);
  float3 x_207 = float3(0.0f, 0.0f, 0.0f);
  float4 x_212 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_217 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x_222 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_227 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  bool x_232 = false;
  bool2 x_238 = bool2(false, false);
  bool3 x_244 = bool3(false, false, false);
  bool4 x_250 = bool4(false, false, false, false);
  int x_256 = 0;
  int2 x_263 = int2(0, 0);
  int3 x_270 = int3(0, 0, 0);
  int4 x_277 = int4(0, 0, 0, 0);
  float4 x_280 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_139 = false;
  bool x_140 = false;
  bool x_145 = false;
  bool x_146 = false;
  bool x_152 = false;
  bool x_153 = false;
  bool x_159 = false;
  bool x_160 = false;
  bool x_166 = false;
  bool x_167 = false;
  bool x_174 = false;
  bool x_175 = false;
  bool x_183 = false;
  bool x_184 = false;
  bool x_193 = false;
  bool x_194 = false;
  bool x_198 = false;
  bool x_199 = false;
  bool x_203 = false;
  bool x_204 = false;
  bool x_208 = false;
  bool x_209 = false;
  bool x_213 = false;
  bool x_214 = false;
  bool x_218 = false;
  bool x_219 = false;
  bool x_223 = false;
  bool x_224 = false;
  bool x_228 = false;
  bool x_229 = false;
  bool x_233 = false;
  bool x_234 = false;
  bool x_239 = false;
  bool x_240 = false;
  bool x_245 = false;
  bool x_246 = false;
  bool x_251 = false;
  bool x_252 = false;
  bool x_257 = false;
  bool x_258 = false;
  bool x_264 = false;
  bool x_265 = false;
  bool x_271 = false;
  bool x_272 = false;
  bool x_278 = false;
  bool x_279 = false;
  x_140 = false;
  if (true) {
    x_139 = takes_void_b();
    x_140 = x_139;
  }
  x_146 = false;
  if (x_140) {
    x_144 = 1.0f;
    x_145 = takes_float_bf(x_144);
    x_146 = x_145;
  }
  x_153 = false;
  if (x_146) {
    x_151 = (2.0f).xx;
    x_152 = takes_float2_bf2(x_151);
    x_153 = x_152;
  }
  x_160 = false;
  if (x_153) {
    x_158 = (3.0f).xxx;
    x_159 = takes_float3_bf3(x_158);
    x_160 = x_159;
  }
  x_167 = false;
  if (x_160) {
    x_165 = (4.0f).xxxx;
    x_166 = takes_float4_bf4(x_165);
    x_167 = x_166;
  }
  x_175 = false;
  if (x_167) {
    x_173 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    x_174 = takes_float2x2_bf22(x_173);
    x_175 = x_174;
  }
  x_184 = false;
  if (x_175) {
    x_182 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    x_183 = takes_float3x3_bf33(x_182);
    x_184 = x_183;
  }
  x_194 = false;
  if (x_184) {
    x_192 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    x_193 = takes_float4x4_bf44(x_192);
    x_194 = x_193;
  }
  x_199 = false;
  if (x_194) {
    x_197 = 1.0f;
    x_198 = takes_half_bh(x_197);
    x_199 = x_198;
  }
  x_204 = false;
  if (x_199) {
    x_202 = (2.0f).xx;
    x_203 = takes_half2_bh2(x_202);
    x_204 = x_203;
  }
  x_209 = false;
  if (x_204) {
    x_207 = (3.0f).xxx;
    x_208 = takes_half3_bh3(x_207);
    x_209 = x_208;
  }
  x_214 = false;
  if (x_209) {
    x_212 = (4.0f).xxxx;
    x_213 = takes_half4_bh4(x_212);
    x_214 = x_213;
  }
  x_219 = false;
  if (x_214) {
    x_217 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
    x_218 = takes_half2x2_bh22(x_217);
    x_219 = x_218;
  }
  x_224 = false;
  if (x_219) {
    x_222 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
    x_223 = takes_half3x3_bh33(x_222);
    x_224 = x_223;
  }
  x_229 = false;
  if (x_224) {
    x_227 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
    x_228 = takes_half4x4_bh44(x_227);
    x_229 = x_228;
  }
  x_234 = false;
  if (x_229) {
    x_232 = true;
    x_233 = takes_bool_bb(x_232);
    x_234 = x_233;
  }
  x_240 = false;
  if (x_234) {
    x_238 = (true).xx;
    x_239 = takes_bool2_bb2(x_238);
    x_240 = x_239;
  }
  x_246 = false;
  if (x_240) {
    x_244 = (true).xxx;
    x_245 = takes_bool3_bb3(x_244);
    x_246 = x_245;
  }
  x_252 = false;
  if (x_246) {
    x_250 = (true).xxxx;
    x_251 = takes_bool4_bb4(x_250);
    x_252 = x_251;
  }
  x_258 = false;
  if (x_252) {
    x_256 = 1;
    x_257 = takes_int_bi(x_256);
    x_258 = x_257;
  }
  x_265 = false;
  if (x_258) {
    x_263 = (2).xx;
    x_264 = takes_int2_bi2(x_263);
    x_265 = x_264;
  }
  x_272 = false;
  if (x_265) {
    x_270 = (3).xxx;
    x_271 = takes_int3_bi3(x_270);
    x_272 = x_271;
  }
  x_279 = false;
  if (x_272) {
    x_277 = (4).xxxx;
    x_278 = takes_int4_bi4(x_277);
    x_279 = x_278;
  }
  if (x_279) {
    const float4 x_287 = asfloat(x_33[0]);
    x_280 = x_287;
  } else {
    const float4 x_289 = asfloat(x_33[1]);
    x_280 = x_289;
  }
  const float4 x_290 = x_280;
  return x_290;
}

void _entrypoint_1() {
  float2 x_43 = float2(0.0f, 0.0f);
  x_43 = (0.0f).xx;
  const float4 x_45 = main(x_43);
  sk_FragColor = x_45;
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
