struct strided_arr {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float foo_ff2(inout float2 x_26) {
  const float2 x_28 = x_26;
  const float2 x_30 = x_26;
  return (x_28.x * x_30.y);
}

void bar_vf(inout float x_35) {
  strided_arr y[2] = (strided_arr[2])0;
  float2 x_55 = float2(0.0f, 0.0f);
  const float x_42 = x_35;
  y[0].el = x_42;
  const float x_45 = x_35;
  y[1].el = (x_45 * 2.0f);
  const float x_51 = y[0].el;
  const float x_53 = y[1].el;
  x_55 = float2(x_51, x_53);
  const float x_56 = foo_ff2(x_55);
  x_35 = x_56;
  return;
}

float4 main(inout float2 x_58) {
  float x = 0.0f;
  float x_62 = 0.0f;
  float4 x_67 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x = 10.0f;
  x_62 = 10.0f;
  bar_vf(x_62);
  const float x_64 = x_62;
  x = x_64;
  if ((x_64 == 200.0f)) {
    const float4 x_74 = asfloat(x_12[0]);
    x_67 = x_74;
  } else {
    const float4 x_76 = asfloat(x_12[1]);
    x_67 = x_76;
  }
  const float4 x_77 = x_67;
  return x_77;
}

void _entrypoint_1() {
  float2 x_22 = float2(0.0f, 0.0f);
  x_22 = (0.0f).xx;
  const float4 x_24 = main(x_22);
  sk_FragColor = x_24;
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
