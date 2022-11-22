static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 x_111 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_83 = false;
  bool x_84 = false;
  bool x_92 = false;
  bool x_93 = false;
  bool x_101 = false;
  bool x_102 = false;
  bool x_109 = false;
  bool x_110 = false;
  const float4 x_32 = asfloat(x_10[0]);
  const float4 x_36 = asfloat(x_10[1]);
  x_51 = false;
  if ((cos(x_32.x) == x_36.x)) {
    const float4 x_43 = asfloat(x_10[0]);
    const float4 x_46 = asfloat(x_10[1]);
    x_50 = all((cos(float2(x_43.x, x_43.y)) == float2(x_46.x, x_46.y)));
    x_51 = x_50;
  }
  x_65 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    const float4 x_60 = asfloat(x_10[1]);
    x_64 = all((cos(float3(x_56.x, x_56.y, x_56.z)) == float3(x_60.x, x_60.y, x_60.z)));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    const float4 x_70 = asfloat(x_10[0]);
    const float4 x_72 = asfloat(x_10[1]);
    x_75 = all((cos(x_70) == x_72));
    x_76 = x_75;
  }
  x_84 = false;
  if (x_76) {
    const float4 x_81 = asfloat(x_10[1]);
    x_83 = (1.0f == x_81.x);
    x_84 = x_83;
  }
  x_93 = false;
  if (x_84) {
    const float4 x_89 = asfloat(x_10[1]);
    x_92 = all(((1.0f).xx == float2(x_89.x, x_89.y)));
    x_93 = x_92;
  }
  x_102 = false;
  if (x_93) {
    const float4 x_98 = asfloat(x_10[1]);
    x_101 = all(((1.0f).xxx == float3(x_98.x, x_98.y, x_98.z)));
    x_102 = x_101;
  }
  x_110 = false;
  if (x_102) {
    const float4 x_107 = asfloat(x_10[1]);
    x_109 = all(((1.0f).xxxx == x_107));
    x_110 = x_109;
  }
  if (x_110) {
    const float4 x_118 = asfloat(x_10[2]);
    x_111 = x_118;
  } else {
    const float4 x_121 = asfloat(x_10[3]);
    x_111 = x_121;
  }
  const float4 x_122 = x_111;
  return x_122;
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
