struct strided_arr {
  float el;
};
struct strided_arr_1 {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static strided_arr globalArray[5] = (strided_arr[5])0;
static float2x2 globalMatrix = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_23 : register(b0, space0) {
  uint4 x_23[8];
};

float2x2 arr_to_mat2x2_stride_16(strided_arr_1 arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

strided_arr tint_symbol_4(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const strided_arr tint_symbol_22 = {asfloat(buffer[scalar_offset / 4][scalar_offset % 4])};
  return tint_symbol_22;
}

typedef strided_arr tint_symbol_3_ret[5];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[8], uint offset) {
  strided_arr arr_1[5] = (strided_arr[5])0;
  {
    for(uint i = 0u; (i < 5u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_4(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

strided_arr_1 tint_symbol_8(uint4 buffer[8], uint offset) {
  const uint scalar_offset_1 = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset_1 / 4];
  const strided_arr_1 tint_symbol_23 = {asfloat(((scalar_offset_1 & 2) ? ubo_load.zw : ubo_load.xy))};
  return tint_symbol_23;
}

typedef strided_arr_1 tint_symbol_7_ret[2];
tint_symbol_7_ret tint_symbol_7(uint4 buffer[8], uint offset) {
  strided_arr_1 arr_2[2] = (strided_arr_1[2])0;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      arr_2[i_1] = tint_symbol_8(buffer, (offset + (i_1 * 16u)));
    }
  }
  return arr_2;
}

float4 main(inout float2 x_36) {
  strided_arr localArray[5] = (strided_arr[5])0;
  float2x2 localMatrix = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_77 = false;
  bool x_78 = false;
  bool x_91 = false;
  bool x_92 = false;
  bool x_110 = false;
  bool x_111 = false;
  bool x_118 = false;
  bool x_119 = false;
  bool x_130 = false;
  bool x_131 = false;
  const strided_arr tint_symbol_10 = {1.0f};
  const strided_arr tint_symbol_11 = {1.0f};
  const strided_arr tint_symbol_12 = {1.0f};
  const strided_arr tint_symbol_13 = {1.0f};
  const strided_arr tint_symbol_14 = {1.0f};
  const strided_arr tint_symbol_15[5] = {tint_symbol_10, tint_symbol_11, tint_symbol_12, tint_symbol_13, tint_symbol_14};
  globalArray = tint_symbol_15;
  globalMatrix = float2x2((1.0f).xx, (1.0f).xx);
  const strided_arr tint_symbol_16 = (strided_arr)0;
  const strided_arr tint_symbol_17 = {1.0f};
  const strided_arr tint_symbol_18 = {2.0f};
  const strided_arr tint_symbol_19 = {3.0f};
  const strided_arr tint_symbol_20 = {4.0f};
  const strided_arr tint_symbol_21[5] = {tint_symbol_16, tint_symbol_17, tint_symbol_18, tint_symbol_19, tint_symbol_20};
  localArray = tint_symbol_21;
  localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
  const strided_arr x_53[5] = tint_symbol_3(x_23, 48u);
  x_78 = true;
  if (((1.0f == x_53[4u].el) & ((1.0f == x_53[3u].el) & ((1.0f == x_53[2u].el) & ((1.0f == x_53[1u].el) & (1.0f == x_53[0u].el)))))) {
  } else {
    const float4 x_73 = asfloat(x_23[0]);
    x_77 = all(((1.0f).xx == float2(x_73.x, x_73.y)));
    x_78 = x_77;
  }
  x_92 = true;
  if (x_78) {
  } else {
    const float2x2 x_84 = arr_to_mat2x2_stride_16(tint_symbol_7(x_23, 16u));
    x_91 = (all(((1.0f).xx == x_84[0u])) & all(((1.0f).xx == x_84[1u])));
    x_92 = x_91;
  }
  x_111 = true;
  if (x_92) {
  } else {
    const strided_arr x_96[5] = tint_symbol_3(x_23, 48u);
    x_110 = ((4.0f == x_96[4u].el) & ((3.0f == x_96[3u].el) & ((2.0f == x_96[2u].el) & ((1.0f == x_96[1u].el) & (0.0f == x_96[0u].el)))));
    x_111 = x_110;
  }
  x_119 = true;
  if (x_111) {
  } else {
    const float4 x_115 = asfloat(x_23[0]);
    x_118 = all(((1.0f).xx == float2(x_115.x, x_115.y)));
    x_119 = x_118;
  }
  x_131 = true;
  if (x_119) {
  } else {
    const float2x2 x_123 = arr_to_mat2x2_stride_16(tint_symbol_7(x_23, 16u));
    x_130 = (all((float2(0.0f, 1.0f) == x_123[0u])) & all((float2(2.0f, 3.0f) == x_123[1u])));
    x_131 = x_130;
  }
  if (x_131) {
    const float4 x_135 = asfloat(x_23[0]);
    return x_135;
  }
  return float4(0.0f, 1.0f, 0.0f, 1.0f);
}

void _entrypoint_1() {
  float2 x_32 = float2(0.0f, 0.0f);
  x_32 = (0.0f).xx;
  const float4 x_34 = main(x_32);
  sk_FragColor = x_34;
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
  const _entrypoint_out tint_symbol_24 = {sk_FragColor};
  return tint_symbol_24;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
