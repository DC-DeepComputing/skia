struct strided_arr {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[5];
};

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

strided_arr tint_symbol_4(uint4 buffer[5], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const strided_arr tint_symbol_7 = {asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy))};
  return tint_symbol_7;
}

typedef strided_arr tint_symbol_3_ret[2];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[5], uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_4(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

float4 main(inout float2 x_25) {
  float4 inputVal = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_82 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_58 = false;
  bool x_59 = false;
  bool x_70 = false;
  bool x_71 = false;
  bool x_80 = false;
  bool x_81 = false;
  const float2x2 x_33 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 16u));
  const float4 x_42 = (float4(x_33[0u].x, x_33[0u].y, x_33[1u].x, x_33[1u].y) * float4(1.0f, 1.0f, -1.0f, -1.0f));
  inputVal = x_42;
  x_59 = false;
  if ((asuint(x_42.x) == 1065353216u)) {
    x_58 = all((asuint(float2(x_42.x, x_42.y)) == uint2(1065353216u, 1073741824u)));
    x_59 = x_58;
  }
  x_71 = false;
  if (x_59) {
    x_70 = all((asuint(float3(x_42.x, x_42.y, x_42.z)) == uint3(1065353216u, 1073741824u, 3225419776u)));
    x_71 = x_70;
  }
  x_81 = false;
  if (x_71) {
    x_80 = all((asuint(x_42) == uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u)));
    x_81 = x_80;
  }
  if (x_81) {
    const float4 x_89 = asfloat(x_10[3]);
    x_82 = x_89;
  } else {
    const float4 x_92 = asfloat(x_10[4]);
    x_82 = x_92;
  }
  const float4 x_93 = x_82;
  return x_93;
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
