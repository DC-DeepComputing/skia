static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_105 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_53 = false;
  bool x_54 = false;
  bool x_67 = false;
  bool x_68 = false;
  bool x_78 = false;
  bool x_79 = false;
  bool x_83 = false;
  bool x_84 = false;
  bool x_90 = false;
  bool x_91 = false;
  bool x_97 = false;
  bool x_98 = false;
  bool x_103 = false;
  bool x_104 = false;
  expected = float4(-1.5625f, 0.0f, 0.75f, 3.375f);
  const float4 x_38 = asfloat(x_10[0]);
  x_54 = false;
  if ((pow(x_38.x, 2.0f) == -1.5625f)) {
    const float4 x_46 = asfloat(x_10[0]);
    x_53 = all((pow(float2(x_46.x, x_46.y), float2(2.0f, 3.0f)) == float2(-1.5625f, 0.0f)));
    x_54 = x_53;
  }
  x_68 = false;
  if (x_54) {
    const float4 x_59 = asfloat(x_10[0]);
    x_67 = all((pow(float3(x_59.x, x_59.y, x_59.z), float3(2.0f, 3.0f, 1.0f)) == float3(-1.5625f, 0.0f, 0.75f)));
    x_68 = x_67;
  }
  x_79 = false;
  if (x_68) {
    const float4 x_73 = asfloat(x_10[0]);
    x_78 = all((pow(x_73, float4(2.0f, 3.0f, 1.0f, 1.5f)) == float4(-1.5625f, 0.0f, 0.75f, 3.375f)));
    x_79 = x_78;
  }
  x_84 = false;
  if (x_79) {
    x_83 = false;
    x_84 = x_83;
  }
  x_91 = false;
  if (x_84) {
    x_90 = false;
    x_91 = x_90;
  }
  x_98 = false;
  if (x_91) {
    x_97 = false;
    x_98 = x_97;
  }
  x_104 = false;
  if (x_98) {
    x_103 = false;
    x_104 = x_103;
  }
  if (x_104) {
    const float4 x_111 = asfloat(x_10[1]);
    x_105 = x_111;
  } else {
    const float4 x_114 = asfloat(x_10[2]);
    x_105 = x_114;
  }
  const float4 x_115 = x_105;
  return x_115;
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
