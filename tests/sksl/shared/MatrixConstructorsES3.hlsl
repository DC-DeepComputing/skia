struct strided_arr {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

strided_arr tint_symbol_4(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const strided_arr tint_symbol_7 = {asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy))};
  return tint_symbol_7;
}

typedef strided_arr tint_symbol_3_ret[2];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[4], uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_4(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

float4 main(inout float2 x_25) {
  float4 f4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool ok = false;
  float4 x_176 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_86 = false;
  bool x_109 = false;
  bool x_110 = false;
  bool x_142 = false;
  bool x_143 = false;
  bool x_174 = false;
  bool x_175 = false;
  const float2x2 x_33 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
  const float x_34 = x_33[0u].x;
  const float x_35 = x_33[0u].y;
  const float x_36 = x_33[1u].x;
  const float x_37 = x_33[1u].y;
  const float4 x_38 = float4(x_34, x_35, x_36, x_37);
  f4 = x_38;
  const float2 x_41 = float2(x_38.x, x_38.y);
  const float3 x_43 = float3(x_34, x_35, x_36);
  const float3 x_46 = float3(x_37, x_41.x, x_41.y);
  const float2x3 x_48 = float2x3(x_43, x_46);
  const bool x_61 = (all((x_43 == float3(1.0f, 2.0f, 3.0f))) & all((x_46 == float3(4.0f, 1.0f, 2.0f))));
  ok = x_61;
  x_86 = false;
  if (x_61) {
    const float3 x_65 = float3(x_38.x, x_38.y, x_38.z);
    const float4 x_66 = float4(x_38.w, x_38.x, x_38.y, x_38.z);
    const float4 x_71 = float4(x_65.x, x_65.y, x_65.z, x_66.x);
    const float4 x_75 = float4(x_66.y, x_66.z, x_66.w, x_37);
    const float2x4 x_77 = float2x4(x_71, x_75);
    x_85 = (all((x_71 == float4(1.0f, 2.0f, 3.0f, 4.0f))) & all((x_75 == float4(1.0f, 2.0f, 3.0f, 4.0f))));
    x_86 = x_85;
  }
  ok = x_86;
  x_110 = false;
  if (x_86) {
    const float2 x_89 = float2(x_38.x, x_38.y);
    const float2 x_90 = float2(x_38.z, x_38.w);
    const float3 x_94 = float3(x_89.x, x_89.y, x_90.x);
    const float3 x_96 = float3(x_90.y, x_34, x_35);
    const float3 x_97 = float3(x_36, x_37, x_34);
    const float3x3 x_99 = float3x3(x_94, x_96, x_97);
    x_109 = ((all((x_94 == float3(1.0f, 2.0f, 3.0f))) & all((x_96 == float3(4.0f, 1.0f, 2.0f)))) & all((x_97 == float3(3.0f, 4.0f, 1.0f))));
    x_110 = x_109;
  }
  ok = x_110;
  x_143 = false;
  if (x_110) {
    const float3 x_113 = float3(x_38.x, x_38.y, x_38.z);
    const float4 x_114 = float4(x_38.w, x_38.x, x_38.y, x_38.z);
    const float2 x_117 = float2(x_113.x, x_113.y);
    const float2 x_120 = float2(x_113.z, x_114.x);
    const float2 x_123 = float2(x_114.y, x_114.z);
    const float2 x_125 = float2(x_114.w, x_37);
    const float4x2 x_127 = float4x2(x_117, x_120, x_123, x_125);
    x_142 = (((all((x_117 == float2(1.0f, 2.0f))) & all((x_120 == float2(3.0f, 4.0f)))) & all((x_123 == float2(1.0f, 2.0f)))) & all((x_125 == float2(3.0f, 4.0f))));
    x_143 = x_142;
  }
  ok = x_143;
  x_175 = false;
  if (x_143) {
    const float4 x_146 = float4(x_38.y, x_38.z, x_38.w, x_38.x);
    const float4 x_147 = float4(x_38.y, x_38.z, x_38.w, x_38.x);
    const float3 x_148 = float3(x_38.y, x_38.z, x_38.w);
    const float3 x_151 = float3(x_34, x_146.x, x_146.y);
    const float3 x_155 = float3(x_146.z, x_146.w, x_147.x);
    const float3 x_159 = float3(x_147.y, x_147.z, x_147.w);
    const float4x3 x_161 = float4x3(x_151, x_155, x_159, x_148);
    x_174 = (((all((x_151 == float3(1.0f, 2.0f, 3.0f))) & all((x_155 == float3(4.0f, 1.0f, 2.0f)))) & all((x_159 == float3(3.0f, 4.0f, 1.0f)))) & all((x_148 == float3(2.0f, 3.0f, 4.0f))));
    x_175 = x_174;
  }
  ok = x_175;
  if (x_175) {
    const float4 x_183 = asfloat(x_10[0]);
    x_176 = x_183;
  } else {
    const float4 x_186 = asfloat(x_10[1]);
    x_176 = x_186;
  }
  const float4 x_187 = x_176;
  return x_187;
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
  const _entrypoint_out tint_symbol_8 = {sk_FragColor};
  return tint_symbol_8;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
