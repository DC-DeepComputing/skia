static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  int4 intValues = int4(0, 0, 0, 0);
  int4 intGreen = int4(0, 0, 0, 0);
  int4 expectedA = int4(0, 0, 0, 0);
  int4 expectedB = int4(0, 0, 0, 0);
  float4 x_171 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_80 = false;
  bool x_81 = false;
  bool x_91 = false;
  bool x_92 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_104 = false;
  bool x_109 = false;
  bool x_110 = false;
  bool x_116 = false;
  bool x_117 = false;
  bool x_120 = false;
  bool x_125 = false;
  bool x_126 = false;
  bool x_134 = false;
  bool x_135 = false;
  bool x_143 = false;
  bool x_144 = false;
  bool x_149 = false;
  bool x_150 = false;
  bool x_153 = false;
  bool x_159 = false;
  bool x_160 = false;
  bool x_166 = false;
  bool x_167 = false;
  bool x_170 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const float4 x_35 = (x_33 * 100.0f);
  const int4 x_44 = int4(int(x_35.x), int(x_35.y), int(x_35.z), int(x_35.w));
  intValues = x_44;
  const float4 x_48 = asfloat(x_10[1]);
  const float4 x_49 = (x_48 * 100.0f);
  const int4 x_58 = int4(int(x_49.x), int(x_49.y), int(x_49.z), int(x_49.w));
  intGreen = x_58;
  expectedA = int4(50, 50, 75, 225);
  expectedB = int4(0, 100, 75, 225);
  const int x_69 = x_44.x;
  x_81 = false;
  if ((max(x_69, 50) == 50)) {
    x_80 = all((max(int2(x_44.x, x_44.y), (50).xx) == (50).xx));
    x_81 = x_80;
  }
  x_92 = false;
  if (x_81) {
    x_91 = all((max(int3(x_44.x, x_44.y, x_44.z), (50).xxx) == int3(50, 50, 75)));
    x_92 = x_91;
  }
  x_100 = false;
  if (x_92) {
    x_99 = all((max(x_44, (50).xxxx) == int4(50, 50, 75, 225)));
    x_100 = x_99;
  }
  x_104 = false;
  if (x_100) {
    x_104 = true;
  }
  x_110 = false;
  if (x_104) {
    x_109 = true;
    x_110 = x_109;
  }
  x_117 = false;
  if (x_110) {
    x_116 = true;
    x_117 = x_116;
  }
  x_120 = false;
  if (x_117) {
    x_120 = true;
  }
  x_126 = false;
  if (x_120) {
    x_125 = (max(x_69, x_58.x) == 0);
    x_126 = x_125;
  }
  x_135 = false;
  if (x_126) {
    x_134 = all((max(int2(x_44.x, x_44.y), int2(x_58.x, x_58.y)) == int2(0, 100)));
    x_135 = x_134;
  }
  x_144 = false;
  if (x_135) {
    x_143 = all((max(int3(x_44.x, x_44.y, x_44.z), int3(x_58.x, x_58.y, x_58.z)) == int3(0, 100, 75)));
    x_144 = x_143;
  }
  x_150 = false;
  if (x_144) {
    x_149 = all((max(x_44, x_58) == int4(0, 100, 75, 225)));
    x_150 = x_149;
  }
  x_153 = false;
  if (x_150) {
    x_153 = true;
  }
  x_160 = false;
  if (x_153) {
    x_159 = true;
    x_160 = x_159;
  }
  x_167 = false;
  if (x_160) {
    x_166 = true;
    x_167 = x_166;
  }
  x_170 = false;
  if (x_167) {
    x_170 = true;
  }
  if (x_170) {
    const float4 x_177 = asfloat(x_10[1]);
    x_171 = x_177;
  } else {
    const float4 x_180 = asfloat(x_10[2]);
    x_171 = x_180;
  }
  const float4 x_181 = x_171;
  return x_181;
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
