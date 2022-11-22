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
  float2x3 testMatrix2x3 = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_106 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_77 = false;
  bool x_78 = false;
  bool x_104 = false;
  bool x_105 = false;
  testMatrix2x3 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
  const float2x2 x_47 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 0u));
  const float2x2 x_42 = transpose(x_47);
  x_78 = false;
  if ((all((x_42[0u] == float2(1.0f, 3.0f))) & all((x_42[1u] == float2(2.0f, 4.0f))))) {
    const float3x2 x_61 = transpose(float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f)));
    x_77 = ((all((x_61[0u] == float2(1.0f, 4.0f))) & all((x_61[1u] == float2(2.0f, 5.0f)))) & all((x_61[2u] == float2(3.0f, 6.0f))));
    x_78 = x_77;
  }
  x_105 = false;
  if (x_78) {
    const float3x3 x_85 = tint_symbol_6(x_10, 32u);
    const float3x3 x_81 = transpose(x_85);
    x_104 = ((all((x_81[0u] == float3(1.0f, 4.0f, 7.0f))) & all((x_81[1u] == float3(2.0f, 5.0f, 8.0f)))) & all((x_81[2u] == float3(3.0f, 6.0f, 9.0f))));
    x_105 = x_104;
  }
  if (x_105) {
    const float4 x_114 = asfloat(x_10[5]);
    x_106 = x_114;
  } else {
    const float4 x_117 = asfloat(x_10[6]);
    x_106 = x_117;
  }
  const float4 x_118 = x_106;
  return x_118;
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
