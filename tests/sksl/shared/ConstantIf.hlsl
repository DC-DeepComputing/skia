static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  float4 x_47 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_40 = false;
  bool x_43 = false;
  bool x_46 = false;
  a = 0;
  b = 0;
  c = 0;
  d = 0;
  a = 1;
  b = 2;
  c = 5;
  x_40 = false;
  if (true) {
    x_40 = true;
  }
  x_43 = false;
  if (x_40) {
    x_43 = true;
  }
  x_46 = false;
  if (x_43) {
    x_46 = true;
  }
  if (x_46) {
    const float4 x_54 = asfloat(x_10[0]);
    x_47 = x_54;
  } else {
    const float4 x_56 = asfloat(x_10[1]);
    x_47 = x_56;
  }
  const float4 x_57 = x_47;
  return x_57;
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
  const _entrypoint_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
