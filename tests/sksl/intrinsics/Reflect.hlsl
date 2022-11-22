static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float expectedX = 0.0f;
  float2 expectedXY = float2(0.0f, 0.0f);
  float3 expectedXYZ = float3(0.0f, 0.0f, 0.0f);
  float4 expectedXYZW = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_110 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_71 = false;
  bool x_72 = false;
  bool x_84 = false;
  bool x_85 = false;
  bool x_95 = false;
  bool x_96 = false;
  bool x_100 = false;
  bool x_103 = false;
  bool x_106 = false;
  bool x_109 = false;
  expectedX = -49.0f;
  expectedXY = float2(-169.0f, 202.0f);
  expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
  expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
  const float4 x_53 = asfloat(x_10[0]);
  const float x_54 = x_53.x;
  const float4 x_57 = asfloat(x_10[1]);
  const float x_58 = x_57.x;
  x_72 = false;
  if (((x_54 - (2.0f * (x_58 * (x_58 * x_54)))) == -49.0f)) {
    const float4 x_64 = asfloat(x_10[0]);
    const float2 x_65 = float2(x_64.x, x_64.y);
    const float4 x_67 = asfloat(x_10[1]);
    const float2 x_68 = float2(x_67.x, x_67.y);
    x_71 = all((reflect(x_65, x_68) == float2(-169.0f, 202.0f)));
    x_72 = x_71;
  }
  x_85 = false;
  if (x_72) {
    const float4 x_77 = asfloat(x_10[0]);
    const float3 x_78 = float3(x_77.x, x_77.y, x_77.z);
    const float4 x_80 = asfloat(x_10[1]);
    const float3 x_81 = float3(x_80.x, x_80.y, x_80.z);
    x_84 = all((reflect(x_78, x_81) == float3(-379.0f, 454.0f, -529.0f)));
    x_85 = x_84;
  }
  x_96 = false;
  if (x_85) {
    const float4 x_90 = asfloat(x_10[0]);
    const float4 x_92 = asfloat(x_10[1]);
    x_95 = all((reflect(x_90, x_92) == float4(-699.0f, 838.0f, -977.0f, 1116.0f)));
    x_96 = x_95;
  }
  x_100 = false;
  if (x_96) {
    x_100 = true;
  }
  x_103 = false;
  if (x_100) {
    x_103 = true;
  }
  x_106 = false;
  if (x_103) {
    x_106 = true;
  }
  x_109 = false;
  if (x_106) {
    x_109 = true;
  }
  if (x_109) {
    const float4 x_116 = asfloat(x_10[2]);
    x_110 = x_116;
  } else {
    const float4 x_119 = asfloat(x_10[3]);
    x_110 = x_119;
  }
  const float4 x_120 = x_110;
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
