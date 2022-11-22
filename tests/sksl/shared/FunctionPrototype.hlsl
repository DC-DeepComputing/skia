static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};

float4 this_function_is_defined_before_use_h4h4(inout float4 x_26) {
  const float4 x_28 = x_26;
  return -(x_28);
}

float4 main(inout float2 x_31) {
  float4 x_39 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_37 = asfloat(x_11[0]);
  x_39 = -(x_37);
  const float4 x_40 = this_function_is_defined_before_use_h4h4(x_39);
  return x_40;
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
