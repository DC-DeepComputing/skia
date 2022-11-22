static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_90 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_54 = false;
  bool x_55 = false;
  bool x_66 = false;
  bool x_67 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_80 = false;
  bool x_83 = false;
  bool x_86 = false;
  bool x_89 = false;
  expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
  const float4 x_38 = asfloat(x_10[0]);
  const float4 x_42 = asfloat(x_10[1]);
  x_55 = false;
  if ((distance(x_38.x, x_42.x) == 3.0f)) {
    const float4 x_49 = asfloat(x_10[0]);
    const float4 x_52 = asfloat(x_10[1]);
    x_54 = (distance(float2(x_49.x, x_49.y), float2(x_52.x, x_52.y)) == 3.0f);
    x_55 = x_54;
  }
  x_67 = false;
  if (x_55) {
    const float4 x_60 = asfloat(x_10[0]);
    const float4 x_64 = asfloat(x_10[1]);
    x_66 = (distance(float3(x_60.x, x_60.y, x_60.z), float3(x_64.x, x_64.y, x_64.z)) == 5.0f);
    x_67 = x_66;
  }
  x_76 = false;
  if (x_67) {
    const float4 x_72 = asfloat(x_10[0]);
    const float4 x_74 = asfloat(x_10[1]);
    x_75 = (distance(x_72, x_74) == 13.0f);
    x_76 = x_75;
  }
  x_80 = false;
  if (x_76) {
    x_80 = true;
  }
  x_83 = false;
  if (x_80) {
    x_83 = true;
  }
  x_86 = false;
  if (x_83) {
    x_86 = true;
  }
  x_89 = false;
  if (x_86) {
    x_89 = true;
  }
  if (x_89) {
    const float4 x_96 = asfloat(x_10[2]);
    x_90 = x_96;
  } else {
    const float4 x_99 = asfloat(x_10[3]);
    x_90 = x_99;
  }
  const float4 x_100 = x_90;
  return x_100;
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
