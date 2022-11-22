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
  uint4 expectedB = uint4(0u, 0u, 0u, 0u);
  float4 x_84 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_64 = false;
  bool x_65 = false;
  bool x_75 = false;
  bool x_76 = false;
  bool x_82 = false;
  bool x_83 = false;
  const float2x2 x_33 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 16u));
  const float4 x_42 = (float4(x_33[0u].x, x_33[0u].y, x_33[1u].x, x_33[1u].y) * float4(1.0f, 1.0f, -1.0f, -1.0f));
  inputVal = x_42;
  expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
  x_65 = false;
  if ((x_42.x == asfloat(1065353216u))) {
    x_64 = all((float2(x_42.x, x_42.y) == asfloat(uint2(1065353216u, 1073741824u))));
    x_65 = x_64;
  }
  x_76 = false;
  if (x_65) {
    x_75 = all((float3(x_42.x, x_42.y, x_42.z) == asfloat(uint3(1065353216u, 1073741824u, 3225419776u))));
    x_76 = x_75;
  }
  x_83 = false;
  if (x_76) {
    x_82 = all((x_42 == asfloat(uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u))));
    x_83 = x_82;
  }
  if (x_83) {
    const float4 x_91 = asfloat(x_10[3]);
    x_84 = x_91;
  } else {
    const float4 x_94 = asfloat(x_10[4]);
    x_84 = x_94;
  }
  const float4 x_95 = x_84;
  return x_95;
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
