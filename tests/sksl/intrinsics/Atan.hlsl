static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 x_188 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_82 = false;
  bool x_83 = false;
  bool x_90 = false;
  bool x_91 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_107 = false;
  bool x_108 = false;
  bool x_119 = false;
  bool x_120 = false;
  bool x_132 = false;
  bool x_133 = false;
  bool x_145 = false;
  bool x_146 = false;
  bool x_156 = false;
  bool x_157 = false;
  bool x_163 = false;
  bool x_164 = false;
  bool x_171 = false;
  bool x_172 = false;
  bool x_179 = false;
  bool x_180 = false;
  bool x_186 = false;
  bool x_187 = false;
  const float4 x_32 = asfloat(x_10[0]);
  const float4 x_36 = asfloat(x_10[1]);
  x_51 = false;
  if ((atan(x_32.x) == x_36.x)) {
    const float4 x_43 = asfloat(x_10[0]);
    const float4 x_46 = asfloat(x_10[1]);
    x_50 = all((atan(float2(x_43.x, x_43.y)) == float2(x_46.x, x_46.y)));
    x_51 = x_50;
  }
  x_65 = false;
  if (x_51) {
    const float4 x_56 = asfloat(x_10[0]);
    const float4 x_60 = asfloat(x_10[1]);
    x_64 = all((atan(float3(x_56.x, x_56.y, x_56.z)) == float3(x_60.x, x_60.y, x_60.z)));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    const float4 x_70 = asfloat(x_10[0]);
    const float4 x_72 = asfloat(x_10[1]);
    x_75 = all((atan(x_70) == x_72));
    x_76 = x_75;
  }
  x_83 = false;
  if (x_76) {
    const float4 x_80 = asfloat(x_10[1]);
    x_82 = (0.0f == x_80.x);
    x_83 = x_82;
  }
  x_91 = false;
  if (x_83) {
    const float4 x_87 = asfloat(x_10[1]);
    x_90 = all(((0.0f).xx == float2(x_87.x, x_87.y)));
    x_91 = x_90;
  }
  x_100 = false;
  if (x_91) {
    const float4 x_96 = asfloat(x_10[1]);
    x_99 = all(((0.0f).xxx == float3(x_96.x, x_96.y, x_96.z)));
    x_100 = x_99;
  }
  x_108 = false;
  if (x_100) {
    const float4 x_105 = asfloat(x_10[1]);
    x_107 = all(((0.0f).xxxx == x_105));
    x_108 = x_107;
  }
  x_120 = false;
  if (x_108) {
    const float4 x_113 = asfloat(x_10[0]);
    const float4 x_117 = asfloat(x_10[1]);
    x_119 = (atan2(x_113.x, 1.0f) == x_117.x);
    x_120 = x_119;
  }
  x_133 = false;
  if (x_120) {
    const float4 x_125 = asfloat(x_10[0]);
    const float4 x_129 = asfloat(x_10[1]);
    x_132 = all((atan2(float2(x_125.x, x_125.y), (1.0f).xx) == float2(x_129.x, x_129.y)));
    x_133 = x_132;
  }
  x_146 = false;
  if (x_133) {
    const float4 x_138 = asfloat(x_10[0]);
    const float4 x_142 = asfloat(x_10[1]);
    x_145 = all((atan2(float3(x_138.x, x_138.y, x_138.z), (1.0f).xxx) == float3(x_142.x, x_142.y, x_142.z)));
    x_146 = x_145;
  }
  x_157 = false;
  if (x_146) {
    const float4 x_151 = asfloat(x_10[0]);
    const float4 x_154 = asfloat(x_10[1]);
    x_156 = all((atan2(x_151, (1.0f).xxxx) == x_154));
    x_157 = x_156;
  }
  x_164 = false;
  if (x_157) {
    const float4 x_161 = asfloat(x_10[1]);
    x_163 = (0.0f == x_161.x);
    x_164 = x_163;
  }
  x_172 = false;
  if (x_164) {
    const float4 x_168 = asfloat(x_10[1]);
    x_171 = all(((0.0f).xx == float2(x_168.x, x_168.y)));
    x_172 = x_171;
  }
  x_180 = false;
  if (x_172) {
    const float4 x_176 = asfloat(x_10[1]);
    x_179 = all(((0.0f).xxx == float3(x_176.x, x_176.y, x_176.z)));
    x_180 = x_179;
  }
  x_187 = false;
  if (x_180) {
    const float4 x_184 = asfloat(x_10[1]);
    x_186 = all(((0.0f).xxxx == x_184));
    x_187 = x_186;
  }
  if (x_187) {
    const float4 x_195 = asfloat(x_10[2]);
    x_188 = x_195;
  } else {
    const float4 x_198 = asfloat(x_10[3]);
    x_188 = x_198;
  }
  const float4 x_199 = x_188;
  return x_199;
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
