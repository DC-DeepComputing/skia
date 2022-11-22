struct strided_arr {
  float el;
};
struct strided_arr_1 {
  float2x2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  strided_arr f[4] = (strided_arr[4])0;
  strided_arr h[4] = (strided_arr[4])0;
  int3 i3[3] = (int3[3])0;
  int3 s3[3] = (int3[3])0;
  strided_arr_1 h2x2[2] = (strided_arr_1[2])0;
  strided_arr_1 f2x2[2] = (strided_arr_1[2])0;
  float4 x_90 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_73 = false;
  bool x_74 = false;
  bool x_88 = false;
  bool x_89 = false;
  const strided_arr tint_symbol_4 = {1.0f};
  const strided_arr tint_symbol_5 = {2.0f};
  const strided_arr tint_symbol_6 = {3.0f};
  const strided_arr tint_symbol_7 = {4.0f};
  const strided_arr x_35[4] = {tint_symbol_4, tint_symbol_5, tint_symbol_6, tint_symbol_7};
  f = x_35;
  h = x_35;
  f = x_35;
  h = x_35;
  const int3 x_47[3] = {(1).xxx, (2).xxx, (3).xxx};
  i3 = x_47;
  s3 = x_47;
  i3 = x_47;
  s3 = x_47;
  const strided_arr_1 tint_symbol_8 = {float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))};
  const strided_arr_1 tint_symbol_9 = {float2x2(float2(5.0f, 6.0f), float2(7.0f, 8.0f))};
  const strided_arr_1 x_63[2] = {tint_symbol_8, tint_symbol_9};
  h2x2 = x_63;
  f2x2 = x_63;
  f2x2 = x_63;
  h2x2 = x_63;
  x_74 = false;
  if (true) {
    x_73 = true;
    x_74 = x_73;
  }
  x_89 = false;
  if (x_74) {
    x_88 = true;
    x_89 = x_88;
  }
  if (x_89) {
    const float4 x_98 = asfloat(x_10[0]);
    x_90 = x_98;
  } else {
    const float4 x_100 = asfloat(x_10[1]);
    x_90 = x_100;
  }
  const float4 x_101 = x_90;
  return x_101;
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
  const _entrypoint_out tint_symbol_10 = {sk_FragColor};
  return tint_symbol_10;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
