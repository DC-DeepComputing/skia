static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_97 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_32 = asfloat(x_10[0]);
  v = x_32;
  const float4 x_35 = float4(x_32.x, 1.0f, 1.0f, 1.0f);
  v = x_35;
  const float2 x_36 = float2(x_35.x, x_35.y);
  const float x_37 = x_36.x;
  v = float4(x_37, x_36.y, 1.0f, 1.0f);
  v = float4(x_37, 1.0f, 1.0f, 1.0f);
  v = float4(0.0f, 1.0f, 1.0f, 1.0f);
  const float3 x_42 = float3(0.0f, 1.0f, 1.0f);
  const float4 x_47 = float4(x_42.x, x_42.y, x_42.z, 1.0f);
  v = x_47;
  const float2 x_48 = float2(x_47.x, x_47.y);
  const float x_49 = x_48.x;
  v = float4(x_49, x_48.y, 1.0f, 1.0f);
  v = float4(x_49, 0.0f, 1.0f, 1.0f);
  const float4 x_53 = float4(x_49, 1.0f, 0.0f, 1.0f);
  v = x_53;
  const float2 x_54 = float2(x_53.y, x_53.z);
  const float x_55 = x_54.x;
  v = float4(1.0f, x_55, x_54.y, 1.0f);
  v = float4(0.0f, x_55, 1.0f, 1.0f);
  v = (1.0f).xxxx;
  const float3 x_60 = (1.0f).xxx;
  const float4 x_64 = float4(x_60.x, x_60.y, x_60.z, 1.0f);
  v = x_64;
  const float2 x_65 = float2(x_64.x, x_64.y);
  const float4 x_68 = float4(x_65.x, x_65.y, 0.0f, 1.0f);
  v = x_68;
  const float2 x_69 = float2(x_68.x, x_68.y);
  const float x_70 = x_69.x;
  const float4 x_72 = float4(x_70, x_69.y, 1.0f, 0.0f);
  v = x_72;
  const float2 x_73 = float2(x_72.z, x_72.w);
  const float x_74 = x_73.x;
  v = float4(x_70, 1.0f, x_74, x_73.y);
  v = float4(x_70, 0.0f, x_74, 1.0f);
  v = float4(x_70, 1.0f, 1.0f, 1.0f);
  const float4 x_79 = float4(x_70, 1.0f, 0.0f, 1.0f);
  v = x_79;
  const float3 x_80 = float3(x_79.y, x_79.z, x_79.w);
  const float4 x_84 = float4(1.0f, x_80.x, x_80.y, x_80.z);
  v = x_84;
  const float2 x_85 = float2(x_84.y, x_84.z);
  const float x_86 = x_85.x;
  v = float4(0.0f, x_86, x_85.y, 1.0f);
  v = float4(0.0f, x_86, 1.0f, 1.0f);
  const float4 x_90 = float4(1.0f, x_86, 1.0f, 1.0f);
  v = x_90;
  const float2 x_91 = float2(x_90.z, x_90.w);
  const float x_92 = x_91.x;
  v = float4(0.0f, 0.0f, x_92, x_91.y);
  v = float4(0.0f, 0.0f, x_92, 1.0f);
  v = float4(0.0f, 1.0f, 1.0f, 1.0f);
  if (true) {
    const float4 x_103 = asfloat(x_10[1]);
    x_97 = x_103;
  } else {
    const float4 x_106 = asfloat(x_10[2]);
    x_97 = x_106;
  }
  const float4 x_107 = x_97;
  return x_107;
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
