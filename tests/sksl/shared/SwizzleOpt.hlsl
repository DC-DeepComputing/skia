static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[3];
};

float fn_hh4(inout float4 x_26) {
  int x = 0;
  x = 1;
  while (true) {
    const int x_37 = x;
    if ((x_37 <= 2)) {
    } else {
      break;
    }
    const float4 x_40 = x_26;
    return x_40.x;
    {
      const int x_42 = x;
      x = (x_42 + 1);
    }
  }
  return 0.0f;
}

float4 main(inout float2 x_45) {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_75 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_81 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_85 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_88 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_91 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_95 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_134 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_50 = asfloat(x_11[2]);
  v = x_50;
  const float3 x_51 = float3(x_50.z, x_50.y, x_50.x);
  const float4 x_56 = float4(0.0f, x_51.x, x_51.y, x_51.z);
  v = x_56;
  const float2 x_57 = float2(x_56.x, x_56.w);
  const float4 x_60 = float4(0.0f, 0.0f, x_57.x, x_57.y);
  v = x_60;
  const float2 x_62 = float2(x_60.w, x_60.x);
  const float4 x_65 = float4(1.0f, 1.0f, x_62.x, x_62.y);
  v = x_65;
  const float2 x_66 = float2(x_65.z, x_65.y);
  const float4 x_69 = float4(x_66.x, x_66.y, 1.0f, 1.0f);
  v = x_69;
  const float2 x_70 = float2(x_69.x, x_69.x);
  const float4 x_73 = float4(x_70.x, x_70.y, 1.0f, 1.0f);
  v = x_73;
  const float4 x_74 = float4(x_73.w, x_73.z, x_73.w, x_73.z);
  v = x_74;
  x_75 = x_74;
  const float x_76 = fn_hh4(x_75);
  const float3 x_79 = float3(x_76, 123.0f, 456.0f);
  const float4 x_80 = float4(x_79.y, x_79.y, x_79.z, x_79.z);
  v = x_80;
  x_81 = x_80;
  const float x_82 = fn_hh4(x_81);
  const float3 x_83 = float3(x_82, 123.0f, 456.0f);
  const float4 x_84 = float4(x_83.y, x_83.y, x_83.z, x_83.z);
  v = x_84;
  x_85 = x_84;
  const float x_86 = fn_hh4(x_85);
  const float4 x_87 = float4(123.0f, 456.0f, 456.0f, x_86);
  v = x_87;
  x_88 = x_87;
  const float x_89 = fn_hh4(x_88);
  const float4 x_90 = float4(123.0f, 456.0f, 456.0f, x_89);
  v = x_90;
  x_91 = x_90;
  const float x_92 = fn_hh4(x_91);
  const float3 x_93 = float3(x_92, 123.0f, 456.0f);
  const float4 x_94 = float4(x_93.y, x_93.x, x_93.x, x_93.z);
  v = x_94;
  x_95 = x_94;
  const float x_96 = fn_hh4(x_95);
  const float3 x_97 = float3(x_96, 123.0f, 456.0f);
  v = float4(x_97.y, x_97.x, x_97.x, x_97.z);
  v = float4(1.0f, 1.0f, 2.0f, 3.0f);
  const float4 x_104 = asfloat(x_11[0]);
  const float3 x_105 = float3(x_104.x, x_104.y, x_104.z);
  v = float4(x_105.x, x_105.y, x_105.z, 1.0f);
  const float4 x_111 = asfloat(x_11[0]);
  const float4 x_114 = asfloat(x_11[0]);
  const float2 x_115 = float2(x_114.y, x_114.z);
  const float4 x_118 = float4(x_111.x, 1.0f, x_115.x, x_115.y);
  v = x_118;
  const float4 x_119 = v;
  const float4 x_120 = float4(x_118.w, x_118.z, x_118.y, x_118.x);
  v = x_120;
  const float2 x_121 = float2(x_120.y, x_120.z);
  const float4 x_122 = v;
  const float4 x_123 = float4(x_121.x, x_122.y, x_122.z, x_121.y);
  v = x_123;
  const float2 x_124 = float2(x_123.w, x_123.w);
  const float3 x_127 = float3(x_124.x, x_124.y, 1.0f);
  const float4 x_128 = v;
  const float4 x_129 = float4(x_127.z, x_127.y, x_127.x, x_128.w);
  v = x_129;
  if (all((x_129 == (1.0f).xxxx))) {
    const float4 x_139 = asfloat(x_11[1]);
    x_134 = x_139;
  } else {
    const float4 x_141 = asfloat(x_11[0]);
    x_134 = x_141;
  }
  const float4 x_142 = x_134;
  return x_142;
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
  const _entrypoint_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
