struct S {
  int i;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float4 fnGreen_h4bf2(inout bool x_27, inout float2 x_28) {
  const float4 x_34 = asfloat(x_12[0]);
  return x_34;
}

float4 fnRed_h4ifS(inout int x_40, inout float x_41, inout S x_42) {
  const float4 x_46 = asfloat(x_12[1]);
  return x_46;
}

float4 main(inout float2 x_48) {
  float4 x_54 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_60 = false;
  float2 x_62 = float2(0.0f, 0.0f);
  int x_65 = 0;
  float x_67 = 0.0f;
  S x_69 = (S)0;
  const float4 x_51 = asfloat(x_12[0]);
  if (!((x_51.y == 0.0f))) {
    x_60 = true;
    const float2 x_61 = x_48;
    x_62 = x_61;
    const float4 x_63 = fnGreen_h4bf2(x_60, x_62);
    x_54 = x_63;
  } else {
    x_65 = 123;
    x_67 = 3.140000105f;
    const S tint_symbol_4 = (S)0;
    x_69 = tint_symbol_4;
    const float4 x_70 = fnRed_h4ifS(x_65, x_67, x_69);
    x_54 = x_70;
  }
  const float4 x_71 = x_54;
  return x_71;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
