static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 x_109 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_82 = false;
  bool x_83 = false;
  bool x_90 = false;
  bool x_91 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_107 = false;
  bool x_108 = false;
  const float4 x_32 = asfloat(x_10[0]);
  const float4 x_36 = asfloat(x_10[1]);
  x_51 = false;
  if ((acos(x_32.x) == x_36.x)) {
    const float4 x_43 = asfloat(x_10[0]);
    const float4 x_46 = asfloat(x_10[1]);
    x_50 = all((acos(float2(x_43.x, x_43.y)) == float2(x_46.x, x_46.y)));
    x_51 = x_50;
  }
  x_65 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    const float4 x_60 = asfloat(x_10[1]);
    x_64 = all((acos(float3(x_56.x, x_56.y, x_56.z)) == float3(x_60.x, x_60.y, x_60.z)));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    const float4 x_70 = asfloat(x_10[0]);
    const float4 x_72 = asfloat(x_10[1]);
    x_75 = all((acos(x_70) == x_72));
    x_76 = x_75;
  }
  x_83 = false;
  if (x_76) {
    const float4 x_80 = asfloat(x_10[1]);
    x_82 = (0.0f == x_80.x);
    x_83 = x_82;
  }
  x_91 = false;
  if (x_83) {
    const float4 x_87 = asfloat(x_10[1]);
    x_90 = all(((0.0f).xx == float2(x_87.x, x_87.y)));
    x_91 = x_90;
  }
  x_100 = false;
  if (x_91) {
    const float4 x_96 = asfloat(x_10[1]);
    x_99 = all(((0.0f).xxx == float3(x_96.x, x_96.y, x_96.z)));
    x_100 = x_99;
  }
  x_108 = false;
  if (x_100) {
    const float4 x_105 = asfloat(x_10[1]);
    x_107 = all(((0.0f).xxxx == x_105));
    x_108 = x_107;
  }
  if (x_108) {
    const float4 x_116 = asfloat(x_10[2]);
    x_109 = x_116;
  } else {
    const float4 x_119 = asfloat(x_10[3]);
    x_109 = x_119;
  }
  const float4 x_120 = x_109;
  return x_120;
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
