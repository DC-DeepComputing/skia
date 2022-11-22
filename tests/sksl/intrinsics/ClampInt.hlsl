int tint_clamp(int e, int low, int high) {
  return min(max(e, low), high);
}

int2 tint_clamp_1(int2 e, int2 low, int2 high) {
  return min(max(e, low), high);
}

int3 tint_clamp_2(int3 e, int3 low, int3 high) {
  return min(max(e, low), high);
}

int4 tint_clamp_3(int4 e, int4 low, int4 high) {
  return min(max(e, low), high);
}

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  int4 intValues = int4(0, 0, 0, 0);
  int4 expectedA = int4(0, 0, 0, 0);
  int4 expectedB = int4(0, 0, 0, 0);
  float4 x_167 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_68 = false;
  bool x_69 = false;
  bool x_80 = false;
  bool x_81 = false;
  bool x_89 = false;
  bool x_90 = false;
  bool x_94 = false;
  bool x_100 = false;
  bool x_101 = false;
  bool x_107 = false;
  bool x_108 = false;
  bool x_111 = false;
  bool x_115 = false;
  bool x_116 = false;
  bool x_127 = false;
  bool x_128 = false;
  bool x_137 = false;
  bool x_138 = false;
  bool x_146 = false;
  bool x_147 = false;
  bool x_150 = false;
  bool x_155 = false;
  bool x_156 = false;
  bool x_162 = false;
  bool x_163 = false;
  bool x_166 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const float4 x_35 = (x_33 * 100.0f);
  const int4 x_44 = int4(int(x_35.x), int(x_35.y), int(x_35.z), int(x_35.w));
  intValues = x_44;
  expectedA = int4(-100, 0, 75, 100);
  expectedB = int4(-100, 0, 50, 225);
  const int x_56 = x_44.x;
  x_69 = false;
  const int tint_symbol_3 = tint_clamp(x_56, -100, 100);
  if ((tint_symbol_3 == -100)) {
    const int2 tint_symbol_4 = tint_clamp_1(int2(x_44.x, x_44.y), (-100).xx, (100).xx);
    x_68 = all((tint_symbol_4 == int2(-100, 0)));
    x_69 = x_68;
  }
  x_81 = false;
  if (x_69) {
    const int3 tint_symbol_5 = tint_clamp_2(int3(x_44.x, x_44.y, x_44.z), (-100).xxx, (100).xxx);
    x_80 = all((tint_symbol_5 == int3(-100, 0, 75)));
    x_81 = x_80;
  }
  x_90 = false;
  if (x_81) {
    const int4 tint_symbol_6 = tint_clamp_3(x_44, (-100).xxxx, (100).xxxx);
    x_89 = all((tint_symbol_6 == int4(-100, 0, 75, 100)));
    x_90 = x_89;
  }
  x_94 = false;
  if (x_90) {
    x_94 = true;
  }
  x_101 = false;
  if (x_94) {
    x_100 = true;
    x_101 = x_100;
  }
  x_108 = false;
  if (x_101) {
    x_107 = true;
    x_108 = x_107;
  }
  x_111 = false;
  if (x_108) {
    x_111 = true;
  }
  x_116 = false;
  if (x_111) {
    const int tint_symbol_7 = tint_clamp(x_56, -100, 100);
    x_115 = (tint_symbol_7 == -100);
    x_116 = x_115;
  }
  x_128 = false;
  if (x_116) {
    const int2 tint_symbol_8 = tint_clamp_1(int2(x_44.x, x_44.y), int2(-100, -200), int2(100, 200));
    x_127 = all((tint_symbol_8 == int2(-100, 0)));
    x_128 = x_127;
  }
  x_138 = false;
  if (x_128) {
    const int3 tint_symbol_9 = tint_clamp_2(int3(x_44.x, x_44.y, x_44.z), int3(-100, -200, -200), int3(100, 200, 50));
    x_137 = all((tint_symbol_9 == int3(-100, 0, 50)));
    x_138 = x_137;
  }
  x_147 = false;
  if (x_138) {
    const int4 tint_symbol_10 = tint_clamp_3(x_44, int4(-100, -200, -200, 100), int4(100, 200, 50, 300));
    x_146 = all((tint_symbol_10 == int4(-100, 0, 50, 225)));
    x_147 = x_146;
  }
  x_150 = false;
  if (x_147) {
    x_150 = true;
  }
  x_156 = false;
  if (x_150) {
    x_155 = true;
    x_156 = x_155;
  }
  x_163 = false;
  if (x_156) {
    x_162 = true;
    x_163 = x_162;
  }
  x_166 = false;
  if (x_163) {
    x_166 = true;
  }
  if (x_166) {
    const float4 x_174 = asfloat(x_10[1]);
    x_167 = x_174;
  } else {
    const float4 x_177 = asfloat(x_10[2]);
    x_167 = x_177;
  }
  const float4 x_178 = x_167;
  return x_178;
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
  const _entrypoint_out tint_symbol_12 = {sk_FragColor};
  return tint_symbol_12;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
