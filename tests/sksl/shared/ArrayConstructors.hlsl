struct strided_arr {
  float el;
};
struct strided_arr_1 {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  strided_arr test1[4] = (strided_arr[4])0;
  strided_arr_1 test2[2] = (strided_arr_1[2])0;
  float4x4 test3[1] = (float4x4[1])0;
  float4 x_71 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const strided_arr tint_symbol_4 = {1.0f};
  const strided_arr tint_symbol_5 = {2.0f};
  const strided_arr tint_symbol_6 = {3.0f};
  const strided_arr tint_symbol_7 = {4.0f};
  const strided_arr tint_symbol_8[4] = {tint_symbol_4, tint_symbol_5, tint_symbol_6, tint_symbol_7};
  test1 = tint_symbol_8;
  const strided_arr_1 tint_symbol_9 = {float2(1.0f, 2.0f)};
  const strided_arr_1 tint_symbol_10 = {float2(3.0f, 4.0f)};
  const strided_arr_1 tint_symbol_11[2] = {tint_symbol_9, tint_symbol_10};
  test2 = tint_symbol_11;
  const float4x4 tint_symbol_12[1] = {float4x4(float4(16.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 16.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 16.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 16.0f))};
  test3 = tint_symbol_12;
  const float x_58 = test1[3].el;
  const float2 x_60 = test2[1].el;
  const float4 x_66 = test3[0][3];
  if ((((x_58 + x_60.y) + x_66.w) == 24.0f)) {
    const float4 x_77 = asfloat(x_10[0]);
    x_71 = x_77;
  } else {
    const float4 x_79 = asfloat(x_10[1]);
    x_71 = x_79;
  }
  const float4 x_80 = x_71;
  return x_80;
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
  const _entrypoint_out tint_symbol_13 = {sk_FragColor};
  return tint_symbol_13;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
