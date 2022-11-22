static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[8];
};

float4 main(inout float2 x_27) {
  float4 x_197 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_90 = false;
  bool x_91 = false;
  bool x_115 = false;
  bool x_116 = false;
  bool x_153 = false;
  bool x_154 = false;
  bool x_169 = false;
  bool x_170 = false;
  bool x_195 = false;
  bool x_196 = false;
  const float2 x_38 = asfloat(x_10[2].xy);
  const float2 x_42 = asfloat(x_10[3].xy);
  const float2x2 x_30 = float2x2(float2((x_42.x * x_38.x), (x_42.x * x_38.y)), float2((x_42.y * x_38.x), (x_42.y * x_38.y)));
  x_91 = false;
  if ((all((x_30[0u] == float2(3.0f, 6.0f))) & all((x_30[1u] == float2(4.0f, 8.0f))))) {
    const float3 x_66 = asfloat(x_10[4].xyz);
    const float3 x_69 = asfloat(x_10[5].xyz);
    const float3x3 x_60 = float3x3(float3((x_69.x * x_66.x), (x_69.x * x_66.y), (x_69.x * x_66.z)), float3((x_69.y * x_66.x), (x_69.y * x_66.y), (x_69.y * x_66.z)), float3((x_69.z * x_66.x), (x_69.z * x_66.y), (x_69.z * x_66.z)));
    x_90 = ((all((x_60[0u] == float3(4.0f, 8.0f, 12.0f))) & all((x_60[1u] == float3(5.0f, 10.0f, 15.0f)))) & all((x_60[2u] == float3(6.0f, 12.0f, 18.0f))));
    x_91 = x_90;
  }
  x_116 = false;
  if (x_91) {
    const float2 x_97 = asfloat(x_10[2].xy);
    const float3 x_100 = asfloat(x_10[5].xyz);
    const float3x2 x_94 = float3x2(float2((x_100.x * x_97.x), (x_100.x * x_97.y)), float2((x_100.y * x_97.x), (x_100.y * x_97.y)), float2((x_100.z * x_97.x), (x_100.z * x_97.y)));
    x_115 = ((all((x_94[0u] == float2(4.0f, 8.0f))) & all((x_94[1u] == float2(5.0f, 10.0f)))) & all((x_94[2u] == float2(6.0f, 12.0f))));
    x_116 = x_115;
  }
  x_154 = false;
  if (x_116) {
    const float4 x_123 = asfloat(x_10[7]);
    const float4x4 x_119 = float4x4(float4((1.0f * x_123.x), (1.0f * x_123.y), (1.0f * x_123.z), (1.0f * x_123.w)), float4((0.0f * x_123.x), (0.0f * x_123.y), (0.0f * x_123.z), (0.0f * x_123.w)), float4((0.0f * x_123.x), (0.0f * x_123.y), (0.0f * x_123.z), (0.0f * x_123.w)), float4((2.0f * x_123.x), (2.0f * x_123.y), (2.0f * x_123.z), (2.0f * x_123.w)));
    x_153 = (((all((x_119[0u] == float4(-1.25f, 0.0f, 0.75f, 2.25f))) & all((x_119[1u] == (0.0f).xxxx))) & all((x_119[2u] == (0.0f).xxxx))) & all((x_119[3u] == float4(-2.5f, 0.0f, 1.5f, 4.5f))));
    x_154 = x_153;
  }
  x_170 = false;
  if (x_154) {
    const float4 x_159 = asfloat(x_10[7]);
    const float2x4 x_157 = float2x4(float4((1.0f * x_159.x), (1.0f * x_159.y), (1.0f * x_159.z), (1.0f * x_159.w)), float4((2.0f * x_159.x), (2.0f * x_159.y), (2.0f * x_159.z), (2.0f * x_159.w)));
    x_169 = (all((x_157[0u] == float4(-1.25f, 0.0f, 0.75f, 2.25f))) & all((x_157[1u] == float4(-2.5f, 0.0f, 1.5f, 4.5f))));
    x_170 = x_169;
  }
  x_196 = false;
  if (x_170) {
    const float4 x_175 = asfloat(x_10[7]);
    const float4x2 x_173 = float4x2(float2((x_175.x * 1.0f), (x_175.x * 2.0f)), float2((x_175.y * 1.0f), (x_175.y * 2.0f)), float2((x_175.z * 1.0f), (x_175.z * 2.0f)), float2((x_175.w * 1.0f), (x_175.w * 2.0f)));
    x_195 = (((all((x_173[0u] == float2(-1.25f, -2.5f))) & all((x_173[1u] == (0.0f).xx))) & all((x_173[2u] == float2(0.75f, 1.5f)))) & all((x_173[3u] == float2(2.25f, 4.5f))));
    x_196 = x_195;
  }
  if (x_196) {
    const float4 x_203 = asfloat(x_10[0]);
    x_197 = x_203;
  } else {
    const float4 x_205 = asfloat(x_10[1]);
    x_197 = x_205;
  }
  const float4 x_206 = x_197;
  return x_206;
}

void _entrypoint_1() {
  float2 x_23 = float2(0.0f, 0.0f);
  x_23 = (0.0f).xx;
  const float4 x_25 = main(x_23);
  sk_FragColor = x_25;
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
