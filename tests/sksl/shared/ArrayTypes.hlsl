struct S {
  float2 v;
};
struct strided_arr {
  S el;
};
struct strided_arr_1 {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

void initialize_vS(inout strided_arr x_27[2]) {
  x_27[0].el.v = float2(0.0f, 1.0f);
  x_27[1].el.v = float2(2.0f, 1.0f);
  return;
}

float4 main(inout float2 x_38) {
  strided_arr_1 x[2] = (strided_arr_1[2])0;
  strided_arr_1 y[2] = (strided_arr_1[2])0;
  strided_arr z[2] = (strided_arr[2])0;
  strided_arr x_52[2] = (strided_arr[2])0;
  x[0].el = (0.0f).xx;
  x[1].el = float2(1.0f, 0.0f);
  y[0].el = float2(0.0f, 1.0f);
  y[1].el = float2(-1.0f, 2.0f);
  initialize_vS(x_52);
  const strided_arr x_54[2] = x_52;
  z = x_54;
  const float2 x_56 = x[0].el;
  const float2 x_59 = x[0].el;
  const float2 x_63 = z[0].el.v;
  const float2 x_67 = x[1].el;
  const float2 x_70 = x[1].el;
  const float2 x_73 = z[0].el.v;
  const float2 x_78 = y[0].el;
  const float2 x_81 = y[0].el;
  const float2 x_85 = z[1].el.v;
  const float2 x_89 = y[1].el;
  const float2 x_92 = y[1].el;
  const float2 x_95 = z[1].el.v;
  return float4(((x_56.x * x_59.y) + x_63.x), (x_67.x - (x_70.y * x_73.y)), ((x_78.x / x_81.y) / x_85.x), (x_89.x + (x_92.y * x_95.y)));
}

void _entrypoint_1() {
  float2 x_18 = float2(0.0f, 0.0f);
  x_18 = (0.0f).xx;
  const float4 x_20 = main(x_18);
  sk_FragColor = x_20;
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
  const _entrypoint_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
