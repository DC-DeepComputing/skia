static float x_kGuardedDivideEpsilon = 0.0f;
static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_18 : register(b0, space0) {
  uint4 x_18[2];
};

float blend_color_saturation_Qhh3(inout float3 x_24) {
  const float3 x_28 = x_24;
  const float3 x_30 = x_24;
  const float3 x_32 = x_24;
  const float3 x_36 = x_24;
  const float3 x_38 = x_24;
  const float3 x_40 = x_24;
  return (max(max(x_28.x, x_30.y), x_32.z) - min(min(x_36.x, x_38.y), x_40.z));
}

float4 blend_hslc_h4h2h4h4(inout float2 x_47, inout float4 x_48, inout float4 x_49) {
  float alpha = 0.0f;
  float3 sda = float3(0.0f, 0.0f, 0.0f);
  float3 dsa = float3(0.0f, 0.0f, 0.0f);
  float3 l = float3(0.0f, 0.0f, 0.0f);
  float3 x_74 = float3(0.0f, 0.0f, 0.0f);
  float3 r = float3(0.0f, 0.0f, 0.0f);
  float3 x_83 = float3(0.0f, 0.0f, 0.0f);
  float x_2_mn = 0.0f;
  float x_3_mx = 0.0f;
  float3 x_103 = float3(0.0f, 0.0f, 0.0f);
  float3 x_109 = float3(0.0f, 0.0f, 0.0f);
  float x_4_lum = 0.0f;
  float3 x_5_result = float3(0.0f, 0.0f, 0.0f);
  float x_6_minComp = 0.0f;
  float x_7_maxComp = 0.0f;
  bool x_144 = false;
  bool x_145 = false;
  bool x_159 = false;
  bool x_160 = false;
  const float4 x_53 = x_49;
  const float4 x_55 = x_48;
  const float x_57 = (x_53.w * x_55.w);
  alpha = x_57;
  const float4 x_59 = x_48;
  const float4 x_61 = x_49;
  const float3 x_63 = (float3(x_59.x, x_59.y, x_59.z) * x_61.w);
  sda = x_63;
  const float4 x_65 = x_49;
  const float4 x_67 = x_48;
  const float3 x_69 = (float3(x_65.x, x_65.y, x_65.z) * x_67.w);
  dsa = x_69;
  const float2 x_71 = x_47;
  if (!((x_71.x == 0.0f))) {
    x_74 = x_69;
  } else {
    x_74 = x_63;
  }
  const float3 x_78 = x_74;
  l = x_78;
  const float2 x_80 = x_47;
  if (!((x_80.x == 0.0f))) {
    x_83 = x_63;
  } else {
    x_83 = x_69;
  }
  float3 x_87 = float3(0.0f, 0.0f, 0.0f);
  x_87 = x_83;
  r = x_87;
  const float2 x_88 = x_47;
  if (!((x_88.y == 0.0f))) {
    const float x_96 = x_78.x;
    const float x_97 = x_78.y;
    const float x_98 = x_78.z;
    const float x_94 = min(min(x_96, x_97), x_98);
    x_2_mn = x_94;
    const float x_100 = max(max(x_96, x_97), x_98);
    x_3_mx = x_100;
    if ((x_100 > x_94)) {
      x_109 = x_87;
      const float x_110 = blend_color_saturation_Qhh3(x_109);
      x_103 = (((x_78 - float3(x_94, x_94, x_94)) * x_110) * (1.0f / (x_100 - x_94)));
    } else {
      x_103 = (0.0f).xxx;
    }
    const float3 x_117 = x_103;
    l = x_117;
    r = x_69;
  }
  const float3 x_124 = r;
  const float x_119 = dot(float3(0.300000012f, 0.589999974f, 0.109999999f), x_124);
  x_4_lum = x_119;
  const float3 x_127 = l;
  const float x_128 = (x_119 - dot(float3(0.300000012f, 0.589999974f, 0.109999999f), x_127));
  const float3 x_129 = l;
  const float3 x_131 = (float3(x_128, x_128, x_128) + x_129);
  x_5_result = x_131;
  const float x_135 = x_131.x;
  const float x_136 = x_131.y;
  const float x_137 = x_131.z;
  const float x_133 = min(min(x_135, x_136), x_137);
  x_6_minComp = x_133;
  const float x_139 = max(max(x_135, x_136), x_137);
  x_7_maxComp = x_139;
  x_145 = false;
  if ((x_133 < 0.0f)) {
    x_144 = !((x_119 == x_133));
    x_145 = x_144;
  }
  if (x_145) {
    const float3 x_148 = float3(x_119, x_119, x_119);
    const float x_151 = x_kGuardedDivideEpsilon;
    x_5_result = (x_148 + ((x_131 - x_148) * (x_119 / ((x_119 - x_133) + x_151))));
  }
  x_160 = false;
  if ((x_139 > x_57)) {
    x_159 = !((x_139 == x_119));
    x_160 = x_159;
  }
  if (x_160) {
    const float3 x_163 = x_5_result;
    const float3 x_164 = float3(x_119, x_119, x_119);
    const float x_169 = x_kGuardedDivideEpsilon;
    x_5_result = (x_164 + (((x_163 - x_164) * (x_57 - x_119)) * (1.0f / ((x_139 - x_119) + x_169))));
  }
  const float3 x_174 = x_5_result;
  const float4 x_175 = x_49;
  const float4 x_179 = x_48;
  const float3 x_182 = ((((x_174 + float3(x_175.x, x_175.y, x_175.z)) - x_69) + float3(x_179.x, x_179.y, x_179.z)) - x_63);
  const float4 x_186 = x_48;
  const float4 x_188 = x_49;
  return float4(x_182.x, x_182.y, x_182.z, ((x_186.w + x_188.w) - x_57));
}

void main_1() {
  float2 x_197 = float2(0.0f, 0.0f);
  float4 x_203 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_207 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_kGuardedDivideEpsilon = 0.0f;
  x_197 = (0.0f).xx;
  const float4 x_202 = asfloat(x_18[0]);
  x_203 = x_202;
  const float4 x_206 = asfloat(x_18[1]);
  x_207 = x_206;
  const float4 x_208 = blend_hslc_h4h2h4h4(x_197, x_203, x_207);
  sk_FragColor = x_208;
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  main_1();
  const main_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
