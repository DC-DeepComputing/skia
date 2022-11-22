static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[3];
};

bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(inout float2 x_37, inout float2 x_38, inout float2 x_39, inout float3 x_40, inout int2 x_41, inout int2 x_42, inout float2 x_43, inout float2 x_44, inout float4 x_45, inout int2 x_46, inout bool4 x_47, inout float2 x_48, inout float2 x_49, inout float2 x_50, inout bool2 x_51, inout bool2 x_52, inout bool3 x_53) {
  const float2 x_55 = x_37;
  const float2 x_57 = x_38;
  const float2 x_60 = x_39;
  const float3 x_63 = x_40;
  const int2 x_66 = x_41;
  const int2 x_70 = x_42;
  const float2 x_74 = x_43;
  const float2 x_77 = x_44;
  const float4 x_80 = x_45;
  const int2 x_83 = x_46;
  const bool4 x_87 = x_47;
  const float2 x_92 = x_48;
  const float2 x_95 = x_49;
  const float2 x_98 = x_50;
  const bool2 x_101 = x_51;
  const bool2 x_105 = x_52;
  const bool3 x_109 = x_53;
  return (((((((((((((((((x_55.x + x_57.x) + x_60.x) + x_63.x) + float(x_66.x)) + float(x_70.x)) + x_74.x) + x_77.x) + x_80.x) + float(x_83.x)) + (x_87.x ? 1.0f : 0.0f)) + x_92.x) + x_95.x) + x_98.x) + (x_101.x ? 1.0f : 0.0f)) + (x_105.x ? 1.0f : 0.0f)) + (x_109.x ? 1.0f : 0.0f)) == 17.0f);
}

float4 main(inout float2 x_116) {
  float2 v1 = float2(0.0f, 0.0f);
  float2 v2 = float2(0.0f, 0.0f);
  float2 v3 = float2(0.0f, 0.0f);
  float3 v4 = float3(0.0f, 0.0f, 0.0f);
  int2 v5 = int2(0, 0);
  int2 v6 = int2(0, 0);
  float2 v7 = float2(0.0f, 0.0f);
  float2 v8 = float2(0.0f, 0.0f);
  float4 v9 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int2 v10 = int2(0, 0);
  bool4 v11 = bool4(false, false, false, false);
  float2 v12 = float2(0.0f, 0.0f);
  float2 v13 = float2(0.0f, 0.0f);
  float2 v14 = float2(0.0f, 0.0f);
  bool2 v15 = bool2(false, false);
  bool2 v16 = bool2(false, false);
  bool3 v17 = bool3(false, false, false);
  float2 x_162 = float2(0.0f, 0.0f);
  float2 x_163 = float2(0.0f, 0.0f);
  float2 x_164 = float2(0.0f, 0.0f);
  float3 x_165 = float3(0.0f, 0.0f, 0.0f);
  int2 x_166 = int2(0, 0);
  int2 x_167 = int2(0, 0);
  float2 x_168 = float2(0.0f, 0.0f);
  float2 x_169 = float2(0.0f, 0.0f);
  float4 x_170 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int2 x_171 = int2(0, 0);
  bool4 x_172 = bool4(false, false, false, false);
  float2 x_173 = float2(0.0f, 0.0f);
  float2 x_174 = float2(0.0f, 0.0f);
  float2 x_175 = float2(0.0f, 0.0f);
  bool2 x_176 = bool2(false, false);
  bool2 x_177 = bool2(false, false);
  bool3 x_178 = bool3(false, false, false);
  float4 x_180 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  v1 = (1.0f).xx;
  v2 = float2(1.0f, 2.0f);
  v3 = (1.0f).xx;
  v4 = (1.0f).xxx;
  v5 = (1).xx;
  v6 = int2(1, 2);
  v7 = float2(1.0f, 2.0f);
  const float2 x_136 = (1.0f).xx;
  v8 = x_136;
  const float x_141 = asfloat(x_11[2].x);
  const float4 x_144 = float4(1.0f, x_141, 3.0f, 4.0f);
  v9 = x_144;
  const int2 x_148 = int2(3, 1);
  v10 = x_148;
  v11 = bool4(true, false, true, false);
  v12 = float2(1.0f, 0.0f);
  v13 = (0.0f).xx;
  v14 = (0.0f).xx;
  v15 = (true).xx;
  v16 = (true).xx;
  v17 = (true).xxx;
  x_162 = (1.0f).xx;
  x_163 = float2(1.0f, 2.0f);
  x_164 = (1.0f).xx;
  x_165 = (1.0f).xxx;
  x_166 = (1).xx;
  x_167 = int2(1, 2);
  x_168 = float2(1.0f, 2.0f);
  x_169 = x_136;
  x_170 = x_144;
  x_171 = x_148;
  x_172 = bool4(true, false, true, false);
  x_173 = float2(1.0f, 0.0f);
  x_174 = (0.0f).xx;
  x_175 = (0.0f).xx;
  x_176 = (true).xx;
  x_177 = (true).xx;
  x_178 = (true).xxx;
  const bool x_179 = check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(x_162, x_163, x_164, x_165, x_166, x_167, x_168, x_169, x_170, x_171, x_172, x_173, x_174, x_175, x_176, x_177, x_178);
  if (x_179) {
    const float4 x_187 = asfloat(x_11[0]);
    x_180 = x_187;
  } else {
    const float4 x_189 = asfloat(x_11[1]);
    x_180 = x_189;
  }
  const float4 x_190 = x_180;
  return x_190;
}

void _entrypoint_1() {
  float2 x_21 = float2(0.0f, 0.0f);
  x_21 = (0.0f).xx;
  const float4 x_23 = main(x_21);
  sk_FragColor = x_23;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
