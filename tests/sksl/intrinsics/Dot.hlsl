static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_91 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_55 = false;
  bool x_56 = false;
  bool x_67 = false;
  bool x_68 = false;
  bool x_76 = false;
  bool x_77 = false;
  bool x_81 = false;
  bool x_84 = false;
  bool x_87 = false;
  bool x_90 = false;
  expected = float4(5.0f, 17.0f, 38.0f, 70.0f);
  const float4 x_39 = asfloat(x_10[0]);
  const float4 x_43 = asfloat(x_10[1]);
  x_56 = false;
  if (((x_39.x * x_43.x) == 5.0f)) {
    const float4 x_50 = asfloat(x_10[0]);
    const float4 x_53 = asfloat(x_10[1]);
    x_55 = (dot(float2(x_50.x, x_50.y), float2(x_53.x, x_53.y)) == 17.0f);
    x_56 = x_55;
  }
  x_68 = false;
  if (x_56) {
    const float4 x_61 = asfloat(x_10[0]);
    const float4 x_65 = asfloat(x_10[1]);
    x_67 = (dot(float3(x_61.x, x_61.y, x_61.z), float3(x_65.x, x_65.y, x_65.z)) == 38.0f);
    x_68 = x_67;
  }
  x_77 = false;
  if (x_68) {
    const float4 x_73 = asfloat(x_10[0]);
    const float4 x_75 = asfloat(x_10[1]);
    x_76 = (dot(x_73, x_75) == 70.0f);
    x_77 = x_76;
  }
  x_81 = false;
  if (x_77) {
    x_81 = true;
  }
  x_84 = false;
  if (x_81) {
    x_84 = true;
  }
  x_87 = false;
  if (x_84) {
    x_87 = true;
  }
  x_90 = false;
  if (x_87) {
    x_90 = true;
  }
  if (x_90) {
    const float4 x_97 = asfloat(x_10[2]);
    x_91 = x_97;
  } else {
    const float4 x_100 = asfloat(x_10[3]);
    x_91 = x_100;
  }
  const float4 x_101 = x_91;
  return x_101;
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
