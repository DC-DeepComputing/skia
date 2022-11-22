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
  float4 x_35 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float2x2 x_32 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 0u));
  if ((determinant(x_32) == -2.0f)) {
    const float4 x_43 = asfloat(x_10[2]);
    x_35 = x_43;
  } else {
    const float4 x_46 = asfloat(x_10[3]);
    x_35 = x_46;
  }
  const float4 x_47 = x_35;
  return x_47;
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
