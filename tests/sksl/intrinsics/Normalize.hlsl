static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expectedVec = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_92 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_48 = false;
  bool x_49 = false;
  bool x_60 = false;
  bool x_61 = false;
  bool x_69 = false;
  bool x_70 = false;
  bool x_74 = false;
  bool x_80 = false;
  bool x_81 = false;
  bool x_87 = false;
  bool x_88 = false;
  bool x_91 = false;
  expectedVec = float4(1.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_36 = asfloat(x_10[0]);
  x_49 = false;
  if (true) {
    const float4 x_43 = asfloat(x_10[0]);
    x_48 = all((normalize(float2(x_43.x, x_43.y)) == float2(1.0f, 0.0f)));
    x_49 = x_48;
  }
  x_61 = false;
  if (x_49) {
    const float4 x_54 = asfloat(x_10[0]);
    x_60 = all((normalize(float3(x_54.x, x_54.y, x_54.z)) == float3(1.0f, 0.0f, 0.0f)));
    x_61 = x_60;
  }
  x_70 = false;
  if (x_61) {
    const float4 x_66 = asfloat(x_10[0]);
    x_69 = all((normalize(x_66) == float4(1.0f, 0.0f, 0.0f, 0.0f)));
    x_70 = x_69;
  }
  x_74 = false;
  if (x_70) {
    x_74 = true;
  }
  x_81 = false;
  if (x_74) {
    x_80 = true;
    x_81 = x_80;
  }
  x_88 = false;
  if (x_81) {
    x_87 = true;
    x_88 = x_87;
  }
  x_91 = false;
  if (x_88) {
    x_91 = true;
  }
  if (x_91) {
    const float4 x_98 = asfloat(x_10[1]);
    x_92 = x_98;
  } else {
    const float4 x_101 = asfloat(x_10[2]);
    x_92 = x_101;
  }
  const float4 x_102 = x_92;
  return x_102;
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
