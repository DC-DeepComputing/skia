struct strided_arr {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 main(inout float2 x_21) {
  strided_arr rgb[3] = (strided_arr[3])0;
  float a = 0.0f;
  rgb[0].el = 0.0f;
  rgb[1].el = 1.0f;
  rgb[2].el = 0.0f;
  a = 1.0f;
  const float x_38 = rgb[0].el;
  const float x_40 = rgb[1].el;
  const float x_42 = rgb[2].el;
  return float4(x_38, x_40, x_42, 1.0f);
}

void _entrypoint_1() {
  float2 x_17 = float2(0.0f, 0.0f);
  x_17 = (0.0f).xx;
  const float4 x_19 = main(x_17);
  sk_FragColor = x_19;
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
