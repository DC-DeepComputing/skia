static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float x = 0.0f;
cbuffer cbuffer_x_14 : register(b0, space0) {
  uint4 x_14[2];
};

bool out_params_are_distinct_from_global_bh(inout float x_29) {
  bool x_38 = false;
  x_29 = 2.0f;
  const float x_33 = x;
  x_38 = false;
  if ((x_33 == 1.0f)) {
    x_38 = true;
  }
  return x_38;
}

float4 main(inout float2 x_40) {
  float x_42 = 0.0f;
  float4 x_45 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x = 1.0f;
  const bool x_43 = out_params_are_distinct_from_global_bh(x_42);
  const float x_44 = x_42;
  x = x_44;
  if (x_43) {
    const float4 x_54 = asfloat(x_14[0]);
    x_45 = x_54;
  } else {
    const float4 x_57 = asfloat(x_14[1]);
    x_45 = x_57;
  }
  const float4 x_58 = x_45;
  return x_58;
}

void _entrypoint_1() {
  float2 x_24 = float2(0.0f, 0.0f);
  x_24 = (0.0f).xx;
  const float4 x_26 = main(x_24);
  sk_FragColor = x_26;
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
