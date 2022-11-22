static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 x_114 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_83 = false;
  bool x_84 = false;
  bool x_93 = false;
  bool x_94 = false;
  bool x_103 = false;
  bool x_104 = false;
  bool x_112 = false;
  bool x_113 = false;
  const float4 x_32 = asfloat(x_10[0]);
  const float4 x_36 = asfloat(x_10[1]);
  x_51 = false;
  if ((exp2(x_32.x) == x_36.x)) {
    const float4 x_43 = asfloat(x_10[0]);
    const float4 x_46 = asfloat(x_10[1]);
    x_50 = all((exp2(float2(x_43.x, x_43.y)) == float2(x_46.x, x_46.y)));
    x_51 = x_50;
  }
  x_65 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    const float4 x_60 = asfloat(x_10[1]);
    x_64 = all((exp2(float3(x_56.x, x_56.y, x_56.z)) == float3(x_60.x, x_60.y, x_60.z)));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    const float4 x_70 = asfloat(x_10[0]);
    const float4 x_72 = asfloat(x_10[1]);
    x_75 = all((exp2(x_70) == x_72));
    x_76 = x_75;
  }
  x_84 = false;
  if (x_76) {
    const float4 x_81 = asfloat(x_10[1]);
    x_83 = (1.0f == x_81.x);
    x_84 = x_83;
  }
  x_94 = false;
  if (x_84) {
    const float4 x_90 = asfloat(x_10[1]);
    x_93 = all((float2(1.0f, 2.0f) == float2(x_90.x, x_90.y)));
    x_94 = x_93;
  }
  x_104 = false;
  if (x_94) {
    const float4 x_100 = asfloat(x_10[1]);
    x_103 = all((float3(1.0f, 2.0f, 4.0f) == float3(x_100.x, x_100.y, x_100.z)));
    x_104 = x_103;
  }
  x_113 = false;
  if (x_104) {
    const float4 x_110 = asfloat(x_10[1]);
    x_112 = all((float4(1.0f, 2.0f, 4.0f, 8.0f) == x_110));
    x_113 = x_112;
  }
  if (x_113) {
    const float4 x_121 = asfloat(x_10[2]);
    x_114 = x_121;
  } else {
    const float4 x_124 = asfloat(x_10[3]);
    x_114 = x_124;
  }
  const float4 x_125 = x_114;
  return x_125;
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
