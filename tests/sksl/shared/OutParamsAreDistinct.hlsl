static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

bool out_params_are_distinct_bhh(inout float x_26, inout float x_27) {
  bool x_35 = false;
  x_26 = 1.0f;
  x_27 = 2.0f;
  x_35 = false;
  if (true) {
    x_35 = true;
  }
  return x_35;
}

float4 main(inout float2 x_37) {
  float x = 0.0f;
  float x_40 = 0.0f;
  float x_41 = 0.0f;
  float4 x_45 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x = 0.0f;
  const bool x_42 = out_params_are_distinct_bhh(x_40, x_41);
  const float x_43 = x_40;
  x = x_43;
  const float x_44 = x_41;
  x = x_44;
  if (x_42) {
    const float4 x_54 = asfloat(x_11[0]);
    x_45 = x_54;
  } else {
    const float4 x_57 = asfloat(x_11[1]);
    x_45 = x_57;
  }
  const float4 x_58 = x_45;
  return x_58;
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
