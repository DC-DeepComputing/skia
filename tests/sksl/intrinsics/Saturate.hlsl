static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_97 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_80 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_92 = false;
  bool x_93 = false;
  bool x_96 = false;
  expected = float4(0.0f, 0.0f, 0.75f, 1.0f);
  const float4 x_37 = asfloat(x_10[0]);
  x_51 = false;
  if ((clamp(x_37.x, 0.0f, 1.0f) == 0.0f)) {
    const float4 x_44 = asfloat(x_10[0]);
    x_50 = all((clamp(float2(x_44.x, x_44.y), (0.0f).xx, (1.0f).xx) == (0.0f).xx));
    x_51 = x_50;
  }
  x_65 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    x_64 = all((clamp(float3(x_56.x, x_56.y, x_56.z), (0.0f).xxx, (1.0f).xxx) == float3(0.0f, 0.0f, 0.75f)));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    const float4 x_70 = asfloat(x_10[0]);
    x_75 = all((clamp(x_70, (0.0f).xxxx, (1.0f).xxxx) == float4(0.0f, 0.0f, 0.75f, 1.0f)));
    x_76 = x_75;
  }
  x_80 = false;
  if (x_76) {
    x_80 = true;
  }
  x_86 = false;
  if (x_80) {
    x_85 = true;
    x_86 = x_85;
  }
  x_93 = false;
  if (x_86) {
    x_92 = true;
    x_93 = x_92;
  }
  x_96 = false;
  if (x_93) {
    x_96 = true;
  }
  if (x_96) {
    const float4 x_103 = asfloat(x_10[1]);
    x_97 = x_103;
  } else {
    const float4 x_106 = asfloat(x_10[2]);
    x_97 = x_106;
  }
  const float4 x_107 = x_97;
  return x_107;
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
