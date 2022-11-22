static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

int tint_mod(int lhs, int rhs) {
  return (lhs % (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

float4 main(inout float2 x_24) {
  float x = 0.0f;
  float y = 0.0f;
  int z = 0;
  bool b = false;
  bool c = false;
  bool d = false;
  bool e = false;
  bool f = false;
  int2 w = int2(0, 0);
  float4 x_141 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_67 = false;
  bool x_68 = false;
  bool x_69 = false;
  bool x_79 = false;
  bool x_83 = false;
  bool x_129 = false;
  bool x_130 = false;
  bool x_133 = false;
  bool x_134 = false;
  bool x_137 = false;
  bool x_140 = false;
  x = 1.0f;
  y = 2.0f;
  z = 3;
  const float x_40 = 2.0f;
  x = x_40;
  const float x_42 = ((x_40 / 2.0f) / x_40);
  y = x_42;
  const int tint_symbol_3 = tint_mod(1, 3);
  const int x_50 = (((tint_symbol_3 << (asuint(4) & 31u)) >> (asuint(2) & 31u)) << (asuint(1) & 31u));
  z = x_50;
  x_69 = true;
  if (((x_40 > 4.0f) == (x_40 < 2.0f))) {
  } else {
    const float x_63 = asfloat(x_10[2].x);
    x_68 = false;
    if ((2.0f >= x_63)) {
      x_67 = (x_42 <= x_40);
      x_68 = x_67;
    }
    x_69 = x_68;
  }
  b = x_69;
  const float x_72 = asfloat(x_10[2].x);
  const bool x_73 = (x_72 > 2.0f);
  c = x_73;
  const bool x_75 = (x_69 != x_73);
  d = x_75;
  x_79 = false;
  if (x_69) {
    x_79 = x_73;
  }
  e = x_79;
  x_83 = true;
  if (x_69) {
  } else {
    x_83 = x_73;
  }
  f = x_83;
  const float x_85 = (x_40 + 12.0f);
  x = x_85;
  const float x_86 = (x_85 - 12.0f);
  x = x_86;
  const float x_88 = (x_42 / 10.0f);
  y = x_88;
  x = (x_86 * x_88);
  const int x_91 = (x_50 | 0);
  z = x_91;
  const int x_93 = (x_91 & -1);
  z = x_93;
  const int x_94 = (x_93 ^ 0);
  z = x_94;
  const int x_95 = (x_94 >> (asuint(2) & 31u));
  z = x_95;
  const int x_96 = (x_95 << (asuint(4) & 31u));
  z = x_96;
  z = tint_mod(x_96, 5);
  const float4 x_101 = asfloat(x_10[0]);
  const float2 x_102 = float2(x_101.x, x_101.y);
  const float x_104 = 6.0f;
  x = x_104;
  const float x_113 = (((((x_69 ? 1.0f : 0.0f) * (x_73 ? 1.0f : 0.0f)) * (x_75 ? 1.0f : 0.0f)) * (x_79 ? 1.0f : 0.0f)) * (x_83 ? 1.0f : 0.0f));
  y = 6.0f;
  const float4 x_116 = asfloat(x_10[1]);
  const float2 x_117 = float2(x_116.z, x_116.w);
  z = 6;
  const int x_121 = -6;
  const int2 x_122 = int2(x_121, x_121);
  w = x_122;
  const int2 x_123 = ~(x_122);
  w = x_123;
  x_130 = false;
  if ((x_123.x == 5)) {
    x_129 = (x_123.y == 5);
    x_130 = x_129;
  }
  x_134 = false;
  if (x_130) {
    x_133 = (x_104 == 6.0f);
    x_134 = x_133;
  }
  x_137 = false;
  if (x_134) {
    x_137 = true;
  }
  x_140 = false;
  if (x_137) {
    x_140 = true;
  }
  if (x_140) {
    const float4 x_147 = asfloat(x_10[0]);
    x_141 = x_147;
  } else {
    const float4 x_149 = asfloat(x_10[1]);
    x_141 = x_149;
  }
  const float4 x_150 = x_141;
  return x_150;
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
  const _entrypoint_out tint_symbol_6 = {sk_FragColor};
  return tint_symbol_6;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
