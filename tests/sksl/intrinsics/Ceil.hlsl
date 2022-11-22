static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_94 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_62 = false;
  bool x_63 = false;
  bool x_71 = false;
  bool x_72 = false;
  bool x_76 = false;
  bool x_82 = false;
  bool x_83 = false;
  bool x_89 = false;
  bool x_90 = false;
  bool x_93 = false;
  expected = float4(-1.0f, 0.0f, 1.0f, 3.0f);
  const float4 x_38 = asfloat(x_10[0]);
  x_51 = false;
  if ((ceil(x_38.x) == -1.0f)) {
    const float4 x_45 = asfloat(x_10[0]);
    x_50 = all((ceil(float2(x_45.x, x_45.y)) == float2(-1.0f, 0.0f)));
    x_51 = x_50;
  }
  x_63 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    x_62 = all((ceil(float3(x_56.x, x_56.y, x_56.z)) == float3(-1.0f, 0.0f, 1.0f)));
    x_63 = x_62;
  }
  x_72 = false;
  if (x_63) {
    const float4 x_68 = asfloat(x_10[0]);
    x_71 = all((ceil(x_68) == float4(-1.0f, 0.0f, 1.0f, 3.0f)));
    x_72 = x_71;
  }
  x_76 = false;
  if (x_72) {
    x_76 = true;
  }
  x_83 = false;
  if (x_76) {
    x_82 = true;
    x_83 = x_82;
  }
  x_90 = false;
  if (x_83) {
    x_89 = true;
    x_90 = x_89;
  }
  x_93 = false;
  if (x_90) {
    x_93 = true;
  }
  if (x_93) {
    const float4 x_100 = asfloat(x_10[1]);
    x_94 = x_100;
  } else {
    const float4 x_103 = asfloat(x_10[2]);
    x_94 = x_103;
  }
  const float4 x_104 = x_94;
  return x_104;
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
