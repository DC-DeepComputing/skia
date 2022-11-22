struct strided_arr {
  float el;
};
struct strided_arr_1 {
  float2x2 el;
};
struct S {
  int x;
  int y;
};
struct strided_arr_2 {
  S el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[12];
};

strided_arr tint_symbol_4(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const strided_arr tint_symbol_35 = {asfloat(buffer[scalar_offset / 4][scalar_offset % 4])};
  return tint_symbol_35;
}

typedef strided_arr tint_symbol_3_ret[5];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[12], uint offset) {
  strided_arr arr[5] = (strided_arr[5])0;
  {
    for(uint i = 0u; (i < 5u); i = (i + 1u)) {
      arr[i] = tint_symbol_4(buffer, (offset + (i * 16u)));
    }
  }
  return arr;
}

float4 main(inout float2 x_27) {
  strided_arr f1[5] = (strided_arr[5])0;
  strided_arr f2[5] = (strided_arr[5])0;
  strided_arr f3[5] = (strided_arr[5])0;
  int3 v1[2] = (int3[2])0;
  int3 v2[2] = (int3[2])0;
  int3 v3[2] = (int3[2])0;
  strided_arr_1 m1[3] = (strided_arr_1[3])0;
  strided_arr_1 m2[3] = (strided_arr_1[3])0;
  strided_arr_1 m3[3] = (strided_arr_1[3])0;
  strided_arr_2 s1[3] = (strided_arr_2[3])0;
  strided_arr_2 s2[3] = (strided_arr_2[3])0;
  strided_arr_2 s3[3] = (strided_arr_2[3])0;
  float4 x_282 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_107 = false;
  bool x_108 = false;
  bool x_134 = false;
  bool x_135 = false;
  bool x_153 = false;
  bool x_154 = false;
  bool x_172 = false;
  bool x_173 = false;
  bool x_191 = false;
  bool x_192 = false;
  bool x_210 = false;
  bool x_211 = false;
  bool x_214 = false;
  bool x_215 = false;
  bool x_221 = false;
  bool x_222 = false;
  bool x_242 = false;
  bool x_243 = false;
  bool x_262 = false;
  bool x_263 = false;
  bool x_272 = false;
  bool x_273 = false;
  bool x_280 = false;
  bool x_281 = false;
  const strided_arr tint_symbol_7 = {1.0f};
  const strided_arr tint_symbol_8 = {2.0f};
  const strided_arr tint_symbol_9 = {3.0f};
  const strided_arr tint_symbol_10 = {4.0f};
  const strided_arr tint_symbol_11 = {5.0f};
  const strided_arr x_36[5] = {tint_symbol_7, tint_symbol_8, tint_symbol_9, tint_symbol_10, tint_symbol_11};
  f1 = x_36;
  f2 = x_36;
  const strided_arr tint_symbol_12 = {1.0f};
  const strided_arr tint_symbol_13 = {2.0f};
  const strided_arr tint_symbol_14 = {3.0f};
  const strided_arr tint_symbol_15 = {-4.0f};
  const strided_arr tint_symbol_16 = {5.0f};
  const strided_arr tint_symbol_17[5] = {tint_symbol_12, tint_symbol_13, tint_symbol_14, tint_symbol_15, tint_symbol_16};
  f3 = tint_symbol_17;
  const int3 x_52[2] = {int3(1, 2, 3), int3(4, 5, 6)};
  v1 = x_52;
  v2 = x_52;
  const int3 tint_symbol_18[2] = {int3(1, 2, 3), int3(4, 5, -6)};
  v3 = tint_symbol_18;
  const strided_arr_1 tint_symbol_19 = {float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f))};
  const strided_arr_1 tint_symbol_20 = {float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f))};
  const strided_arr_1 tint_symbol_21 = {float2x2(float2(3.0f, 4.0f), float2(5.0f, 6.0f))};
  const strided_arr_1 x_72[3] = {tint_symbol_19, tint_symbol_20, tint_symbol_21};
  m1 = x_72;
  m2 = x_72;
  const strided_arr_1 tint_symbol_22 = {float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f))};
  const strided_arr_1 tint_symbol_23 = {float2x2(float2(2.0f, 3.0f), float2(4.0f, 5.0f))};
  const strided_arr_1 tint_symbol_24 = {float2x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f))};
  const strided_arr_1 tint_symbol_25[3] = {tint_symbol_22, tint_symbol_23, tint_symbol_24};
  m3 = tint_symbol_25;
  const S x_86 = {1, 2};
  const S x_88 = {5, 6};
  const strided_arr_2 tint_symbol_26 = {x_86};
  const S tint_symbol_27 = {3, 4};
  const strided_arr_2 tint_symbol_28 = {tint_symbol_27};
  const strided_arr_2 tint_symbol_29 = {x_88};
  const strided_arr_2 x_89[3] = {tint_symbol_26, tint_symbol_28, tint_symbol_29};
  s1 = x_89;
  const strided_arr_2 tint_symbol_30 = {x_86};
  const S tint_symbol_31 = (S)0;
  const strided_arr_2 tint_symbol_32 = {tint_symbol_31};
  const strided_arr_2 tint_symbol_33 = {x_88};
  const strided_arr_2 tint_symbol_34[3] = {tint_symbol_30, tint_symbol_32, tint_symbol_33};
  s2 = tint_symbol_34;
  s3 = x_89;
  x_108 = false;
  if (true) {
    x_107 = true;
    x_108 = x_107;
  }
  x_135 = false;
  if (x_108) {
    const strided_arr x_113[5] = tint_symbol_3(x_10, 32u);
    const strided_arr x_115[5] = tint_symbol_3(x_10, 112u);
    x_134 = (!((x_113[4u].el == x_115[4u].el)) | (!((x_113[3u].el == x_115[3u].el)) | (!((x_113[2u].el == x_115[2u].el)) | (!((x_113[1u].el == x_115[1u].el)) | !((x_113[0u].el == x_115[0u].el))))));
    x_135 = x_134;
  }
  x_154 = false;
  if (x_135) {
    const strided_arr x_139[5] = tint_symbol_3(x_10, 32u);
    x_153 = ((x_139[4u].el == 5.0f) & ((x_139[3u].el == 4.0f) & ((x_139[2u].el == 3.0f) & ((x_139[1u].el == 2.0f) & (x_139[0u].el == 1.0f)))));
    x_154 = x_153;
  }
  x_173 = false;
  if (x_154) {
    const strided_arr x_158[5] = tint_symbol_3(x_10, 32u);
    x_172 = (!((x_158[4u].el == 5.0f)) | (!((x_158[3u].el == -4.0f)) | (!((x_158[2u].el == 3.0f)) | (!((x_158[1u].el == 2.0f)) | !((x_158[0u].el == 1.0f))))));
    x_173 = x_172;
  }
  x_192 = false;
  if (x_173) {
    const strided_arr x_177[5] = tint_symbol_3(x_10, 32u);
    x_191 = ((5.0f == x_177[4u].el) & ((4.0f == x_177[3u].el) & ((3.0f == x_177[2u].el) & ((2.0f == x_177[1u].el) & (1.0f == x_177[0u].el)))));
    x_192 = x_191;
  }
  x_211 = false;
  if (x_192) {
    const strided_arr x_196[5] = tint_symbol_3(x_10, 32u);
    x_210 = (!((5.0f == x_196[4u].el)) | (!((-4.0f == x_196[3u].el)) | (!((3.0f == x_196[2u].el)) | (!((2.0f == x_196[1u].el)) | !((1.0f == x_196[0u].el))))));
    x_211 = x_210;
  }
  x_215 = false;
  if (x_211) {
    x_214 = true;
    x_215 = x_214;
  }
  x_222 = false;
  if (x_215) {
    x_221 = true;
    x_222 = x_221;
  }
  x_243 = false;
  if (x_222) {
    x_242 = true;
    x_243 = x_242;
  }
  x_263 = false;
  if (x_243) {
    x_262 = true;
    x_263 = x_262;
  }
  x_273 = false;
  if (x_263) {
    x_272 = true;
    x_273 = x_272;
  }
  x_281 = false;
  if (x_273) {
    x_280 = true;
    x_281 = x_280;
  }
  if (x_281) {
    const float4 x_289 = asfloat(x_10[0]);
    x_282 = x_289;
  } else {
    const float4 x_291 = asfloat(x_10[1]);
    x_282 = x_291;
  }
  const float4 x_292 = x_282;
  return x_292;
}

void _entrypoint_1() {
  float2 x_23 = float2(0.0f, 0.0f);
  x_23 = (0.0f).xx;
  const float4 x_25 = main(x_23);
  sk_FragColor = x_25;
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
  const _entrypoint_out tint_symbol_36 = {sk_FragColor};
  return tint_symbol_36;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
