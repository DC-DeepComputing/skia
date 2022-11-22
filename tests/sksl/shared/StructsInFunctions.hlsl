struct S {
  float x;
  int y;
};
struct Nested {
  S a;
  S b;
};
struct Compound {
  float4 f4;
  int3 i3;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_14 : register(b0, space0) {
  uint4 x_14[2];
};

S returns_a_struct_S() {
  S s = (S)0;
  s.x = 1.0f;
  s.y = 2;
  const S x_41 = s;
  return x_41;
}

float accepts_a_struct_fS(inout S x_47) {
  const float x_50 = x_47.x;
  const int x_52 = x_47.y;
  return (x_50 + float(x_52));
}

void modifies_a_struct_vS(inout S x_56) {
  const float x_59 = x_56.x;
  x_56.x = (x_59 + 1.0f);
  const int x_62 = x_56.y;
  x_56.y = (x_62 + 1);
  return;
}

S constructs_a_struct_S() {
  const S tint_symbol_4 = {2.0f, 3};
  return tint_symbol_4;
}

float4 main(inout float2 x_65) {
  S s_1 = (S)0;
  float x = 0.0f;
  S x_70 = (S)0;
  S x_72 = (S)0;
  S expected = (S)0;
  Nested n1 = (Nested)0;
  Nested n2 = (Nested)0;
  Nested n3 = (Nested)0;
  S x_90 = (S)0;
  Compound c1 = (Compound)0;
  Compound c2 = (Compound)0;
  Compound c3 = (Compound)0;
  bool valid = false;
  float4 x_251 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_127 = false;
  bool x_128 = false;
  bool x_133 = false;
  bool x_134 = false;
  bool x_145 = false;
  bool x_146 = false;
  bool x_155 = false;
  bool x_156 = false;
  bool x_167 = false;
  bool x_168 = false;
  bool x_191 = false;
  bool x_192 = false;
  bool x_215 = false;
  bool x_216 = false;
  bool x_235 = false;
  bool x_236 = false;
  bool x_243 = false;
  bool x_244 = false;
  bool x_249 = false;
  bool x_250 = false;
  const S x_68 = returns_a_struct_S();
  s_1 = x_68;
  x_70 = x_68;
  const float x_71 = accepts_a_struct_fS(x_70);
  x = x_71;
  x_72 = x_68;
  modifies_a_struct_vS(x_72);
  const S x_74 = x_72;
  s_1 = x_74;
  const S x_76 = constructs_a_struct_S();
  expected = x_76;
  const S x_82 = returns_a_struct_S();
  n1.a = x_82;
  const S x_85 = n1.a;
  n1.b = x_85;
  const Nested x_87 = n1;
  n2 = x_87;
  n3 = x_87;
  const S x_89 = n3.b;
  x_90 = x_89;
  modifies_a_struct_vS(x_90);
  const S x_92 = x_90;
  n3.b = x_92;
  const Compound tint_symbol_5 = {float4(1.0f, 2.0f, 3.0f, 4.0f), int3(5, 6, 7)};
  c1 = tint_symbol_5;
  const float4 x_108 = asfloat(x_14[1]);
  const float4 x_110 = float4(x_108.y, 2.0f, 3.0f, 4.0f);
  const Compound tint_symbol_6 = {x_110, int3(5, 6, 7)};
  c2 = tint_symbol_6;
  const float4 x_114 = asfloat(x_14[1]);
  const float4 x_116 = float4(x_114.x, 2.0f, 3.0f, 4.0f);
  const Compound tint_symbol_7 = {x_116, int3(5, 6, 7)};
  c3 = tint_symbol_7;
  const float x_121 = x;
  x_128 = false;
  if ((x_121 == 3.0f)) {
    const float x_126 = s_1.x;
    x_127 = (x_126 == 2.0f);
    x_128 = x_127;
  }
  x_134 = false;
  if (x_128) {
    const int x_132 = s_1.y;
    x_133 = (x_132 == 3);
    x_134 = x_133;
  }
  x_146 = false;
  if (x_134) {
    const S x_137 = s_1;
    const S x_138 = expected;
    x_145 = ((x_137.y == x_138.y) & (x_137.x == x_138.x));
    x_146 = x_145;
  }
  x_156 = false;
  if (x_146) {
    const S x_149 = s_1;
    const S x_150 = {2.0f, 3};
    x_155 = ((x_149.y == 3) & (x_149.x == 2.0f));
    x_156 = x_155;
  }
  x_168 = false;
  if (x_156) {
    const S x_159 = s_1;
    const S x_160 = returns_a_struct_S();
    x_167 = ((x_159.y != x_160.y) | !((x_159.x == x_160.x)));
    x_168 = x_167;
  }
  x_192 = false;
  if (x_168) {
    const Nested x_171 = n1;
    const Nested x_172 = n2;
    const S x_173 = x_171.a;
    const S x_174 = x_172.a;
    const S x_182 = x_171.b;
    const S x_183 = x_172.b;
    x_191 = (((x_182.y == x_183.y) & (x_182.x == x_183.x)) & ((x_173.y == x_174.y) & (x_173.x == x_174.x)));
    x_192 = x_191;
  }
  x_216 = false;
  if (x_192) {
    const Nested x_195 = n1;
    const Nested x_196 = n3;
    const S x_197 = x_195.a;
    const S x_198 = x_196.a;
    const S x_206 = x_195.b;
    const S x_207 = x_196.b;
    x_215 = (((x_206.y != x_207.y) | !((x_206.x == x_207.x))) | ((x_197.y != x_198.y) | !((x_197.x == x_198.x))));
    x_216 = x_215;
  }
  x_236 = false;
  if (x_216) {
    const Nested x_219 = n3;
    const S tint_symbol_8 = {1.0f, 2};
    const S tint_symbol_9 = {2.0f, 3};
    const Nested x_222 = {tint_symbol_8, tint_symbol_9};
    const S x_223 = x_219.a;
    const S x_229 = x_219.b;
    x_235 = (((x_229.y == 3) & (x_229.x == 2.0f)) & ((x_223.y == 2) & (x_223.x == 1.0f)));
    x_236 = x_235;
  }
  x_244 = false;
  if (x_236) {
    x_243 = (true & all((float4(1.0f, 2.0f, 3.0f, 4.0f) == x_110)));
    x_244 = x_243;
  }
  x_250 = false;
  if (x_244) {
    x_249 = (false | any(!((x_110 == x_116))));
    x_250 = x_249;
  }
  valid = x_250;
  if (x_250) {
    const float4 x_257 = asfloat(x_14[1]);
    x_251 = x_257;
  } else {
    const float4 x_259 = asfloat(x_14[0]);
    x_251 = x_259;
  }
  const float4 x_260 = x_251;
  return x_260;
}

void _entrypoint_1() {
  float2 x_24 = float2(0.0f, 0.0f);
  x_24 = (0.0f).xx;
  const float4 x_26 = main(x_24);
  sk_FragColor = x_26;
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
  const _entrypoint_out tint_symbol_10 = {sk_FragColor};
  return tint_symbol_10;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
