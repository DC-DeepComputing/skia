static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

float4 main(inout float2 x_24) {
  float4 expectedPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 expectedNeg = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_125 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_71 = false;
  bool x_72 = false;
  bool x_89 = false;
  bool x_90 = false;
  bool x_102 = false;
  bool x_103 = false;
  bool x_107 = false;
  bool x_113 = false;
  bool x_114 = false;
  bool x_120 = false;
  bool x_121 = false;
  bool x_124 = false;
  expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
  expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
  const float4 x_45 = asfloat(x_10[0]);
  const float x_46 = x_45.x;
  const float4 x_49 = asfloat(x_10[1]);
  const float4 x_53 = asfloat(x_10[2]);
  x_72 = false;
  if (((((x_49.x * x_53.x) < 0.0f) ? x_46 : -(x_46)) == -1.0f)) {
    const float4 x_60 = asfloat(x_10[0]);
    const float2 x_61 = float2(x_60.x, x_60.y);
    const float4 x_63 = asfloat(x_10[1]);
    const float4 x_66 = asfloat(x_10[2]);
    x_71 = all((faceforward(x_61, float2(x_63.x, x_63.y), float2(x_66.x, x_66.y)) == float2(-1.0f, -2.0f)));
    x_72 = x_71;
  }
  x_90 = false;
  if (x_72) {
    const float4 x_77 = asfloat(x_10[0]);
    const float3 x_78 = float3(x_77.x, x_77.y, x_77.z);
    const float4 x_81 = asfloat(x_10[1]);
    const float4 x_84 = asfloat(x_10[2]);
    x_89 = all((faceforward(x_78, float3(x_81.x, x_81.y, x_81.z), float3(x_84.x, x_84.y, x_84.z)) == float3(1.0f, 2.0f, 3.0f)));
    x_90 = x_89;
  }
  x_103 = false;
  if (x_90) {
    const float4 x_95 = asfloat(x_10[0]);
    const float4 x_97 = asfloat(x_10[1]);
    const float4 x_99 = asfloat(x_10[2]);
    x_102 = all((faceforward(x_95, x_97, x_99) == float4(1.0f, 2.0f, 3.0f, 4.0f)));
    x_103 = x_102;
  }
  x_107 = false;
  if (x_103) {
    x_107 = true;
  }
  x_114 = false;
  if (x_107) {
    x_113 = true;
    x_114 = x_113;
  }
  x_121 = false;
  if (x_114) {
    x_120 = true;
    x_121 = x_120;
  }
  x_124 = false;
  if (x_121) {
    x_124 = true;
  }
  if (x_124) {
    const float4 x_131 = asfloat(x_10[3]);
    x_125 = x_131;
  } else {
    const float4 x_134 = asfloat(x_10[4]);
    x_125 = x_134;
  }
  const float4 x_135 = x_125;
  return x_135;
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
