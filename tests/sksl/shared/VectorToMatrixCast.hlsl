static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  bool ok = false;
  float4 x_266 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_56 = false;
  bool x_57 = false;
  bool x_73 = false;
  bool x_74 = false;
  bool x_94 = false;
  bool x_95 = false;
  bool x_111 = false;
  bool x_112 = false;
  bool x_147 = false;
  bool x_148 = false;
  bool x_164 = false;
  bool x_165 = false;
  bool x_181 = false;
  bool x_182 = false;
  bool x_217 = false;
  bool x_218 = false;
  bool x_241 = false;
  bool x_242 = false;
  bool x_264 = false;
  bool x_265 = false;
  ok = true;
  x_57 = false;
  if (true) {
    const float4 x_36 = asfloat(x_10[2]);
    const float2 x_41 = float2(x_36.x, x_36.y);
    const float2 x_42 = float2(x_36.z, x_36.w);
    const float2x2 x_44 = float2x2(x_41, x_42);
    x_56 = (all((x_41 == float2(-1.25f, 0.0f))) & all((x_42 == float2(0.75f, 2.25f))));
    x_57 = x_56;
  }
  ok = x_57;
  x_74 = false;
  if (x_57) {
    const float4 x_61 = asfloat(x_10[2]);
    const float2 x_66 = float2(x_61.x, x_61.y);
    const float2 x_67 = float2(x_61.z, x_61.w);
    const float2x2 x_68 = float2x2(x_66, x_67);
    x_73 = (all((x_66 == float2(-1.25f, 0.0f))) & all((x_67 == float2(0.75f, 2.25f))));
    x_74 = x_73;
  }
  ok = x_74;
  x_95 = false;
  if (x_74) {
    const float4 x_79 = asfloat(x_10[0]);
    const float2 x_84 = float2(x_79.x, x_79.y);
    const float2 x_85 = float2(x_79.z, x_79.w);
    const float2x2 x_86 = float2x2(x_84, x_85);
    x_94 = (all((x_84 == float2(0.0f, 1.0f))) & all((x_85 == float2(0.0f, 1.0f))));
    x_95 = x_94;
  }
  ok = x_95;
  x_112 = false;
  if (x_95) {
    const float4 x_99 = asfloat(x_10[0]);
    const float2 x_104 = float2(x_99.x, x_99.y);
    const float2 x_105 = float2(x_99.z, x_99.w);
    const float2x2 x_106 = float2x2(x_104, x_105);
    x_111 = (all((x_104 == float2(0.0f, 1.0f))) & all((x_105 == float2(0.0f, 1.0f))));
    x_112 = x_111;
  }
  ok = x_112;
  x_148 = false;
  if (x_112) {
    const float4 x_116 = asfloat(x_10[0]);
    const int4 x_126 = int4(int(x_116.x), int(x_116.y), int(x_116.z), int(x_116.w));
    const float4 x_135 = float4(float(x_126.x), float(x_126.y), float(x_126.z), float(x_126.w));
    const float2 x_140 = float2(x_135.x, x_135.y);
    const float2 x_141 = float2(x_135.z, x_135.w);
    const float2x2 x_142 = float2x2(x_140, x_141);
    x_147 = (all((x_140 == float2(0.0f, 1.0f))) & all((x_141 == float2(0.0f, 1.0f))));
    x_148 = x_147;
  }
  ok = x_148;
  x_165 = false;
  if (x_148) {
    const float4 x_152 = asfloat(x_10[0]);
    const float2 x_157 = float2(x_152.x, x_152.y);
    const float2 x_158 = float2(x_152.z, x_152.w);
    const float2x2 x_159 = float2x2(x_157, x_158);
    x_164 = (all((x_157 == float2(0.0f, 1.0f))) & all((x_158 == float2(0.0f, 1.0f))));
    x_165 = x_164;
  }
  ok = x_165;
  x_182 = false;
  if (x_165) {
    const float4 x_169 = asfloat(x_10[0]);
    const float2 x_174 = float2(x_169.x, x_169.y);
    const float2 x_175 = float2(x_169.z, x_169.w);
    const float2x2 x_176 = float2x2(x_174, x_175);
    x_181 = (all((x_174 == float2(0.0f, 1.0f))) & all((x_175 == float2(0.0f, 1.0f))));
    x_182 = x_181;
  }
  ok = x_182;
  x_218 = false;
  if (x_182) {
    const float4 x_186 = asfloat(x_10[0]);
    const bool4 x_196 = bool4(!((x_186.x == 0.0f)), !((x_186.y == 0.0f)), !((x_186.z == 0.0f)), !((x_186.w == 0.0f)));
    const float4 x_205 = float4((x_196.x ? 1.0f : 0.0f), (x_196.y ? 1.0f : 0.0f), (x_196.z ? 1.0f : 0.0f), (x_196.w ? 1.0f : 0.0f));
    const float2 x_210 = float2(x_205.x, x_205.y);
    const float2 x_211 = float2(x_205.z, x_205.w);
    const float2x2 x_212 = float2x2(x_210, x_211);
    x_217 = (all((x_210 == float2(0.0f, 1.0f))) & all((x_211 == float2(0.0f, 1.0f))));
    x_218 = x_217;
  }
  ok = x_218;
  x_242 = false;
  if (x_218) {
    const float4 x_222 = asfloat(x_10[0]);
    const float4 x_225 = asfloat(x_10[1]);
    const float4 x_226 = (x_222 - x_225);
    const float2 x_231 = float2(x_226.x, x_226.y);
    const float2 x_232 = float2(x_226.z, x_226.w);
    const float2x2 x_233 = float2x2(x_231, x_232);
    x_241 = (all((x_231 == float2(-1.0f, 1.0f))) & all((x_232 == (0.0f).xx)));
    x_242 = x_241;
  }
  ok = x_242;
  x_265 = false;
  if (x_242) {
    const float4 x_246 = asfloat(x_10[0]);
    const float4 x_249 = (x_246 + (5.0f).xxxx);
    const float2 x_254 = float2(x_249.x, x_249.y);
    const float2 x_255 = float2(x_249.z, x_249.w);
    const float2x2 x_256 = float2x2(x_254, x_255);
    x_264 = (all((x_254 == float2(5.0f, 6.0f))) & all((x_255 == float2(5.0f, 6.0f))));
    x_265 = x_264;
  }
  ok = x_265;
  if (x_265) {
    const float4 x_272 = asfloat(x_10[0]);
    x_266 = x_272;
  } else {
    const float4 x_274 = asfloat(x_10[1]);
    x_266 = x_274;
  }
  const float4 x_275 = x_266;
  return x_275;
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
