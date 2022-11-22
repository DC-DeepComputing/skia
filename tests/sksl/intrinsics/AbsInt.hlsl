static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  int4 expected = int4(0, 0, 0, 0);
  float4 x_118 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_57 = false;
  bool x_58 = false;
  bool x_77 = false;
  bool x_78 = false;
  bool x_95 = false;
  bool x_96 = false;
  bool x_100 = false;
  bool x_106 = false;
  bool x_107 = false;
  bool x_113 = false;
  bool x_114 = false;
  bool x_117 = false;
  expected = int4(1, 0, 0, 2);
  const float4 x_38 = asfloat(x_10[0]);
  x_58 = false;
  if ((abs(int(x_38.x)) == 1)) {
    const float4 x_46 = asfloat(x_10[0]);
    const float2 x_47 = float2(x_46.x, x_46.y);
    x_57 = all((abs(int2(int(x_47.x), int(x_47.y))) == int2(1, 0)));
    x_58 = x_57;
  }
  x_78 = false;
  if (x_58) {
    const float4 x_63 = asfloat(x_10[0]);
    const float3 x_64 = float3(x_63.x, x_63.y, x_63.z);
    x_77 = all((abs(int3(int(x_64.x), int(x_64.y), int(x_64.z))) == int3(1, 0, 0)));
    x_78 = x_77;
  }
  x_96 = false;
  if (x_78) {
    const float4 x_83 = asfloat(x_10[0]);
    x_95 = all((abs(int4(int(x_83.x), int(x_83.y), int(x_83.z), int(x_83.w))) == int4(1, 0, 0, 2)));
    x_96 = x_95;
  }
  x_100 = false;
  if (x_96) {
    x_100 = true;
  }
  x_107 = false;
  if (x_100) {
    x_106 = true;
    x_107 = x_106;
  }
  x_114 = false;
  if (x_107) {
    x_113 = true;
    x_114 = x_113;
  }
  x_117 = false;
  if (x_114) {
    x_117 = true;
  }
  if (x_117) {
    const float4 x_124 = asfloat(x_10[1]);
    x_118 = x_124;
  } else {
    const float4 x_126 = asfloat(x_10[2]);
    x_118 = x_126;
  }
  const float4 x_127 = x_118;
  return x_127;
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
