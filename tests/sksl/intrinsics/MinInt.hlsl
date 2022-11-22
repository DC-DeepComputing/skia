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
  float4 x_170 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_79 = false;
  bool x_80 = false;
  bool x_90 = false;
  bool x_91 = false;
  bool x_98 = false;
  bool x_99 = false;
  bool x_103 = false;
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
  bool x_158 = false;
  bool x_159 = false;
  bool x_165 = false;
  bool x_166 = false;
  bool x_169 = false;
  const float4 x_33 = asfloat(x_10[0]);
  const float4 x_35 = (x_33 * 100.0f);
  const int4 x_44 = int4(int(x_35.x), int(x_35.y), int(x_35.z), int(x_35.w));
  intValues = x_44;
  const float4 x_48 = asfloat(x_10[1]);
  const float4 x_49 = (x_48 * 100.0f);
  const int4 x_58 = int4(int(x_49.x), int(x_49.y), int(x_49.z), int(x_49.w));
  intGreen = x_58;
  expectedA = int4(-125, 0, 50, 50);
  expectedB = int4(-125, 0, 0, 100);
  const int x_68 = x_44.x;
  x_80 = false;
  if ((min(x_68, 50) == -125)) {
    x_79 = all((min(int2(x_44.x, x_44.y), (50).xx) == int2(-125, 0)));
    x_80 = x_79;
  }
  x_91 = false;
  if (x_80) {
    x_90 = all((min(int3(x_44.x, x_44.y, x_44.z), (50).xxx) == int3(-125, 0, 50)));
    x_91 = x_90;
  }
  x_99 = false;
  if (x_91) {
    x_98 = all((min(x_44, (50).xxxx) == int4(-125, 0, 50, 50)));
    x_99 = x_98;
  }
  x_103 = false;
  if (x_99) {
    x_103 = true;
  }
  x_110 = false;
  if (x_103) {
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
    x_125 = (min(x_68, x_58.x) == -125);
    x_126 = x_125;
  }
  x_135 = false;
  if (x_126) {
    x_134 = all((min(int2(x_44.x, x_44.y), int2(x_58.x, x_58.y)) == int2(-125, 0)));
    x_135 = x_134;
  }
  x_144 = false;
  if (x_135) {
    x_143 = all((min(int3(x_44.x, x_44.y, x_44.z), int3(x_58.x, x_58.y, x_58.z)) == int3(-125, 0, 0)));
    x_144 = x_143;
  }
  x_150 = false;
  if (x_144) {
    x_149 = all((min(x_44, x_58) == int4(-125, 0, 0, 100)));
    x_150 = x_149;
  }
  x_153 = false;
  if (x_150) {
    x_153 = true;
  }
  x_159 = false;
  if (x_153) {
    x_158 = true;
    x_159 = x_158;
  }
  x_166 = false;
  if (x_159) {
    x_165 = true;
    x_166 = x_165;
  }
  x_169 = false;
  if (x_166) {
    x_169 = true;
  }
  if (x_169) {
    const float4 x_176 = asfloat(x_10[1]);
    x_170 = x_176;
  } else {
    const float4 x_179 = asfloat(x_10[2]);
    x_170 = x_179;
  }
  const float4 x_180 = x_170;
  return x_180;
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
