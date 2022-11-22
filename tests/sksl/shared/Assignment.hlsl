struct strided_arr {
  float el;
};
struct S {
  float f;
  strided_arr af[5];
  float4 h4;
  float4 ah4[5];
};
struct strided_arr_1 {
  int el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 globalVar = float4(0.0f, 0.0f, 0.0f, 0.0f);
static S globalStruct = (S)0;
cbuffer cbuffer_x_22 : register(b0, space0) {
  uint4 x_22[1];
};

void keepAlive_vf(inout float x_39) {
  return;
}

void keepAlive_vh(inout float x_37) {
  return;
}

void keepAlive_vi(inout int x_43) {
  return;
}

float4 main(inout float2 x_46) {
  int i = 0;
  int4 i4 = int4(0, 0, 0, 0);
  float3x3 f3x3 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x = float4(0.0f, 0.0f, 0.0f, 0.0f);
  strided_arr_1 ai[1] = (strided_arr_1[1])0;
  int4 ai4[1] = (int4[1])0;
  float3x3 ah3x3[1] = (float3x3[1])0;
  float4 af4[1] = (float4[1])0;
  S s = (S)0;
  float l = 0.0f;
  float x_131 = 0.0f;
  float x_138 = 0.0f;
  int x_142 = 0;
  int x_147 = 0;
  int x_152 = 0;
  int x_158 = 0;
  float x_163 = 0.0f;
  float x_168 = 0.0f;
  float x_172 = 0.0f;
  float x_178 = 0.0f;
  i = 0;
  i4 = int4(1, 2, 3, 4);
  f3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
  x.w = 0.0f;
  const float4 x_78 = x;
  x = float4(0.0f, 0.0f, x_78.z, x_78.w);
  ai[0].el = 0;
  ai4[0] = int4(1, 2, 3, 4);
  ah3x3[0] = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
  af4[0].x = 0.0f;
  const float4 x_99 = af4[0];
  af4[0] = (1.0f).xxxx;
  s.f = 0.0f;
  s.af[1].el = 0.0f;
  const float4 x_107 = s.h4;
  s.h4 = float4(9.0f, 9.0f, 9.0f, x_107.w);
  const float4 x_111 = s.ah4[2];
  s.ah4[2] = float4(x_111.x, 5.0f, x_111.z, 5.0f);
  globalVar = (0.0f).xxxx;
  globalStruct.f = 0.0f;
  l = 0.0f;
  const int x_118 = ai[0].el;
  const int4 x_120 = ai4[0];
  ai[0].el = (x_118 + x_120.x);
  s.f = 1.0f;
  s.af[0].el = 2.0f;
  s.h4 = (1.0f).xxxx;
  s.ah4[0] = (2.0f).xxxx;
  const float x_130 = af4[0].x;
  x_131 = x_130;
  keepAlive_vf(x_131);
  const float x_133 = x_131;
  af4[0].x = x_133;
  const float x_137 = ah3x3[0][0].x;
  x_138 = x_137;
  keepAlive_vh(x_138);
  const float x_140 = x_138;
  ah3x3[0][0].x = x_140;
  const int x_141 = i;
  x_142 = x_141;
  keepAlive_vi(x_142);
  const int x_144 = x_142;
  i = x_144;
  const int x_146 = i4.y;
  x_147 = x_146;
  keepAlive_vi(x_147);
  const int x_149 = x_147;
  i4.y = x_149;
  const int x_151 = ai[0].el;
  x_152 = x_151;
  keepAlive_vi(x_152);
  const int x_154 = x_152;
  ai[0].el = x_154;
  const int x_157 = ai4[0].x;
  x_158 = x_157;
  keepAlive_vi(x_158);
  const int x_160 = x_158;
  ai4[0].x = x_160;
  const float x_162 = x.y;
  x_163 = x_162;
  keepAlive_vh(x_163);
  const float x_165 = x_163;
  x.y = x_165;
  const float x_167 = s.f;
  x_168 = x_167;
  keepAlive_vf(x_168);
  const float x_170 = x_168;
  s.f = x_170;
  const float x_171 = l;
  x_172 = x_171;
  keepAlive_vh(x_172);
  const float x_174 = x_172;
  l = x_174;
  const float x_177 = f3x3[0].x;
  x_178 = x_177;
  keepAlive_vf(x_178);
  const float x_180 = x_178;
  f3x3[0].x = x_180;
  const float4 x_183 = asfloat(x_22[0]);
  return x_183;
}

void _entrypoint_1() {
  float2 x_32 = float2(0.0f, 0.0f);
  x_32 = (0.0f).xx;
  const float4 x_34 = main(x_32);
  sk_FragColor = x_34;
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
