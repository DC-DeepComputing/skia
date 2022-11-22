static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

bool verify_const_globals_bii(inout int x_27, inout int x_28) {
  bool x_38 = false;
  bool x_39 = false;
  const int x_31 = x_27;
  x_39 = false;
  if ((x_31 == 7)) {
    const int x_36 = x_28;
    x_38 = (x_36 == 10);
    x_39 = x_38;
  }
  return x_39;
}

float4 main(inout float2 x_41) {
  int x_43 = 0;
  int x_44 = 0;
  float4 x_46 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_43 = 7;
  x_44 = 10;
  const bool x_45 = verify_const_globals_bii(x_43, x_44);
  if (x_45) {
    const float4 x_54 = asfloat(x_11[0]);
    x_46 = x_54;
  } else {
    const float4 x_57 = asfloat(x_11[1]);
    x_46 = x_57;
  }
  const float4 x_58 = x_46;
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
