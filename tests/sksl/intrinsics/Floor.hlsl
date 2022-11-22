static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_93 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_49 = false;
  bool x_50 = false;
  bool x_61 = false;
  bool x_62 = false;
  bool x_70 = false;
  bool x_71 = false;
  bool x_75 = false;
  bool x_81 = false;
  bool x_82 = false;
  bool x_88 = false;
  bool x_89 = false;
  bool x_92 = false;
  expected = float4(-2.0f, 0.0f, 0.0f, 2.0f);
  const float4 x_37 = asfloat(x_10[0]);
  x_50 = false;
  if ((floor(x_37.x) == -2.0f)) {
    const float4 x_44 = asfloat(x_10[0]);
    x_49 = all((floor(float2(x_44.x, x_44.y)) == float2(-2.0f, 0.0f)));
    x_50 = x_49;
  }
  x_62 = false;
  if (x_50) {
    const float4 x_55 = asfloat(x_10[0]);
    x_61 = all((floor(float3(x_55.x, x_55.y, x_55.z)) == float3(-2.0f, 0.0f, 0.0f)));
    x_62 = x_61;
  }
  x_71 = false;
  if (x_62) {
    const float4 x_67 = asfloat(x_10[0]);
    x_70 = all((floor(x_67) == float4(-2.0f, 0.0f, 0.0f, 2.0f)));
    x_71 = x_70;
  }
  x_75 = false;
  if (x_71) {
    x_75 = true;
  }
  x_82 = false;
  if (x_75) {
    x_81 = true;
    x_82 = x_81;
  }
  x_89 = false;
  if (x_82) {
    x_88 = true;
    x_89 = x_88;
  }
  x_92 = false;
  if (x_89) {
    x_92 = true;
  }
  if (x_92) {
    const float4 x_99 = asfloat(x_10[1]);
    x_93 = x_99;
  } else {
    const float4 x_102 = asfloat(x_10[2]);
    x_93 = x_102;
  }
  const float4 x_103 = x_93;
  return x_103;
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
