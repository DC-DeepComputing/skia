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
  float4 x_128 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_88 = false;
  bool x_89 = false;
  bool x_126 = false;
  bool x_127 = false;
  const float2x2 x_33 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
  const float x_34 = x_33[0u].x;
  const float x_35 = x_33[0u].y;
  const float x_36 = x_33[1u].x;
  const float x_37 = x_33[1u].y;
  const float4 x_38 = float4(x_34, x_35, x_36, x_37);
  f4 = x_38;
  const float3 x_41 = float3(x_38.x, x_38.y, x_38.z);
  const float2 x_46 = float2(x_41.x, x_41.y);
  const float2 x_48 = float2(x_41.z, 4.0f);
  const float2x2 x_49 = float2x2(x_46, x_48);
  const bool x_61 = (all((x_46 == float2(1.0f, 2.0f))) & all((x_48 == float2(3.0f, 4.0f))));
  ok = x_61;
  x_89 = false;
  if (x_61) {
    const float2 x_65 = float2(x_38.x, x_38.y);
    const float2 x_66 = float2(x_38.z, x_38.w);
    const float3 x_70 = float3(x_65.x, x_65.y, x_66.x);
    const float3 x_72 = float3(x_66.y, x_34, x_35);
    const float3 x_73 = float3(x_36, x_37, x_34);
    const float3x3 x_75 = float3x3(x_70, x_72, x_73);
    x_88 = ((all((x_70 == float3(1.0f, 2.0f, 3.0f))) & all((x_72 == float3(4.0f, 1.0f, 2.0f)))) & all((x_73 == float3(3.0f, 4.0f, 1.0f))));
    x_89 = x_88;
  }
  ok = x_89;
  x_127 = false;
  if (x_89) {
    const float3 x_92 = float3(x_38.x, x_38.y, x_38.z);
    const float3 x_93 = float3(x_38.w, x_38.x, x_38.y);
    const float4 x_94 = float4(x_38.z, x_38.w, x_38.x, x_38.y);
    const float2 x_95 = float2(x_38.z, x_38.w);
    const float4 x_100 = float4(x_92.x, x_92.y, x_92.z, x_93.x);
    const float4 x_105 = float4(x_93.y, x_93.z, x_94.x, x_94.y);
    const float4 x_110 = float4(x_94.z, x_94.w, x_95.x, x_95.y);
    const float4x4 x_112 = float4x4(x_100, x_105, x_110, x_38);
    x_126 = (((all((x_100 == float4(1.0f, 2.0f, 3.0f, 4.0f))) & all((x_105 == float4(1.0f, 2.0f, 3.0f, 4.0f)))) & all((x_110 == float4(1.0f, 2.0f, 3.0f, 4.0f)))) & all((x_38 == float4(1.0f, 2.0f, 3.0f, 4.0f))));
    x_127 = x_126;
  }
  ok = x_127;
  if (x_127) {
    const float4 x_135 = asfloat(x_10[0]);
    x_128 = x_135;
  } else {
    const float4 x_138 = asfloat(x_10[1]);
    x_128 = x_138;
  }
  const float4 x_139 = x_128;
  return x_139;
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
