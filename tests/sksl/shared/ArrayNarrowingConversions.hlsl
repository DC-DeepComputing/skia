struct strided_arr {
  int el;
};
struct strided_arr_1 {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  strided_arr i2[2] = (strided_arr[2])0;
  strided_arr s2[2] = (strided_arr[2])0;
  strided_arr_1 f2[2] = (strided_arr_1[2])0;
  strided_arr_1 h2[2] = (strided_arr_1[2])0;
  strided_arr_1 cf2[2] = (strided_arr_1[2])0;
  float4 x_59 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_49 = false;
  bool x_50 = false;
  bool x_53 = false;
  bool x_54 = false;
  bool x_57 = false;
  bool x_58 = false;
  const strided_arr tint_symbol_4 = {1};
  const strided_arr tint_symbol_5 = {2};
  const strided_arr tint_symbol_6[2] = {tint_symbol_4, tint_symbol_5};
  i2 = tint_symbol_6;
  const strided_arr tint_symbol_7 = {1};
  const strided_arr tint_symbol_8 = {2};
  const strided_arr x_34[2] = {tint_symbol_7, tint_symbol_8};
  s2 = x_34;
  const strided_arr_1 tint_symbol_9 = {1.0f};
  const strided_arr_1 tint_symbol_10 = {2.0f};
  const strided_arr_1 x_40[2] = {tint_symbol_9, tint_symbol_10};
  f2 = x_40;
  const strided_arr_1 tint_symbol_11 = {1.0f};
  const strided_arr_1 tint_symbol_12 = {2.0f};
  const strided_arr_1 x_42[2] = {tint_symbol_11, tint_symbol_12};
  h2 = x_42;
  i2 = x_34;
  s2 = x_34;
  f2 = x_42;
  h2 = x_42;
  cf2 = x_40;
  x_50 = false;
  if (true) {
    x_49 = true;
    x_50 = x_49;
  }
  x_54 = false;
  if (x_50) {
    x_53 = true;
    x_54 = x_53;
  }
  x_58 = false;
  if (x_54) {
    x_57 = true;
    x_58 = x_57;
  }
  if (x_58) {
    const float4 x_67 = asfloat(x_10[0]);
    x_59 = x_67;
  } else {
    const float4 x_69 = asfloat(x_10[1]);
    x_59 = x_69;
  }
  const float4 x_70 = x_59;
  return x_70;
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
