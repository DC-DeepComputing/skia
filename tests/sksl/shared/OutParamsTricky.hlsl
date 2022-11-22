static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float2 tricky_h2hhh2h(inout float x_27, inout float x_28, inout float2 x_29, inout float x_30) {
  const float2 x_32 = x_29;
  x_29 = float2(x_32.y, x_32.x);
  const float x_34 = x_27;
  const float x_35 = x_28;
  const float x_37 = x_30;
  return float2((x_34 + x_35), x_37);
}

void func_vh4(inout float4 x_41) {
  float2 t = float2(0.0f, 0.0f);
  float x_45 = 0.0f;
  float x_47 = 0.0f;
  float2 x_50 = float2(0.0f, 0.0f);
  float x_52 = 0.0f;
  x_45 = 1.0f;
  x_47 = 2.0f;
  const float4 x_48 = x_41;
  x_50 = float2(x_48.x, x_48.z);
  x_52 = 5.0f;
  const float2 x_53 = tricky_h2hhh2h(x_45, x_47, x_50, x_52);
  const float2 x_54 = x_50;
  const float4 x_55 = x_41;
  x_41 = float4(x_54.x, x_55.y, x_54.y, x_55.w);
  t = x_53;
  const float4 x_57 = x_41;
  x_41 = float4(x_57.x, x_53.x, x_57.z, x_53.y);
  return;
}

float4 main(inout float2 x_60) {
  float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_65 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_72 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  result = float4(0.0f, 1.0f, 2.0f, 3.0f);
  x_65 = float4(0.0f, 1.0f, 2.0f, 3.0f);
  func_vh4(x_65);
  const float4 x_67 = x_65;
  result = x_67;
  if (all((x_67 == float4(2.0f, 3.0f, 0.0f, 5.0f)))) {
    const float4 x_80 = asfloat(x_12[0]);
    x_72 = x_80;
  } else {
    const float4 x_83 = asfloat(x_12[1]);
    x_72 = x_83;
  }
  const float4 x_84 = x_72;
  return x_84;
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
