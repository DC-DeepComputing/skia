static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

float4 main(inout float2 x_24) {
  float2x4 h24 = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x2 h42 = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 f43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_144 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_127 = false;
  bool x_128 = false;
  bool x_142 = false;
  bool x_143 = false;
  const float4 x_37 = asfloat(x_10[1]);
  const float4 x_40 = asfloat(x_10[0]);
  const float2x4 x_41 = float2x4(x_37, x_40);
  const float4 x_42 = ((9.0f).xxxx * x_37);
  const float4 x_43 = ((9.0f).xxxx * x_40);
  h24 = float2x4(x_42, x_43);
  const float4 x_63 = asfloat(x_10[1]);
  const float4 x_65 = asfloat(x_10[0]);
  const float2 x_68 = float2(x_63.x, x_63.y);
  const float2 x_71 = float2(x_63.z, x_63.w);
  const float2 x_74 = float2(x_65.x, x_65.y);
  const float2 x_77 = float2(x_65.z, x_65.w);
  const float4x2 x_78 = float4x2(x_68, x_71, x_74, x_77);
  const float2 x_79 = (float2(1.0f, 2.0f) * x_68);
  const float2 x_80 = (float2(3.0f, 4.0f) * x_71);
  const float2 x_81 = (float2(5.0f, 6.0f) * x_74);
  const float2 x_82 = (float2(7.0f, 8.0f) * x_77);
  h42 = float4x2(x_79, x_80, x_81, x_82);
  f43 = float4x3(float3(12.0f, 22.0f, 30.0f), float3(36.0f, 40.0f, 42.0f), float3(42.0f, 40.0f, 36.0f), float3(30.0f, 22.0f, 12.0f));
  x_128 = false;
  if ((all((x_42 == float4(9.0f, 0.0f, 0.0f, 9.0f))) & all((x_43 == float4(0.0f, 9.0f, 0.0f, 9.0f))))) {
    x_127 = (((all((x_79 == float2(1.0f, 0.0f))) & all((x_80 == float2(0.0f, 4.0f)))) & all((x_81 == float2(0.0f, 6.0f)))) & all((x_82 == float2(0.0f, 8.0f))));
    x_128 = x_127;
  }
  x_143 = false;
  if (x_128) {
    x_142 = true;
    x_143 = x_142;
  }
  if (x_143) {
    const float4 x_150 = asfloat(x_10[0]);
    x_144 = x_150;
  } else {
    const float4 x_152 = asfloat(x_10[1]);
    x_144 = x_152;
  }
  const float4 x_153 = x_144;
  return x_153;
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
