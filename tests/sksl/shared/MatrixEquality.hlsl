struct strided_arr {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[7];
};

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

strided_arr tint_symbol_4(uint4 buffer[7], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const strided_arr tint_symbol_9 = {asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy))};
  return tint_symbol_9;
}

typedef strided_arr tint_symbol_3_ret[2];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[7], uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_4(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

float3x3 tint_symbol_6(uint4 buffer[7], uint offset) {
  const uint scalar_offset_1 = ((offset + 0u)) / 4;
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  const uint scalar_offset_3 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

float4 main(inout float2 x_27) {
  bool x_0_ok = false;
  float4 x_120 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_54 = false;
  bool x_55 = false;
  bool x_82 = false;
  bool x_83 = false;
  bool x_98 = false;
  bool x_99 = false;
  bool x_118 = false;
  bool x_119 = false;
  x_0_ok = true;
  x_55 = false;
  if (true) {
    const float2x2 x_39 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    x_54 = (all((x_39[0u] == float2(1.0f, 2.0f))) & all((x_39[1u] == float2(3.0f, 4.0f))));
    x_55 = x_54;
  }
  x_0_ok = x_55;
  x_83 = false;
  if (x_55) {
    const float3x3 x_61 = tint_symbol_6(x_10, 64u);
    x_82 = ((all((x_61[0u] == float3(1.0f, 2.0f, 3.0f))) & all((x_61[1u] == float3(4.0f, 5.0f, 6.0f)))) & all((x_61[2u] == float3(7.0f, 8.0f, 9.0f))));
    x_83 = x_82;
  }
  x_0_ok = x_83;
  x_99 = false;
  if (x_83) {
    const float2x2 x_87 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    x_98 = (any(!((x_87[0u] == float2(100.0f, 0.0f)))) | any(!((x_87[1u] == float2(0.0f, 100.0f)))));
    x_99 = x_98;
  }
  x_0_ok = x_99;
  x_119 = false;
  if (x_99) {
    const float3x3 x_103 = tint_symbol_6(x_10, 64u);
    x_118 = ((any(!((x_103[0u] == float3(9.0f, 8.0f, 7.0f)))) | any(!((x_103[1u] == float3(6.0f, 5.0f, 4.0f))))) | any(!((x_103[2u] == float3(3.0f, 2.0f, 1.0f)))));
    x_119 = x_118;
  }
  x_0_ok = x_119;
  if (x_119) {
    const float4 x_128 = asfloat(x_10[0]);
    x_120 = x_128;
  } else {
    const float4 x_131 = asfloat(x_10[1]);
    x_120 = x_131;
  }
  const float4 x_132 = x_120;
  return x_132;
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
  const _entrypoint_out tint_symbol_10 = {sk_FragColor};
  return tint_symbol_10;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
