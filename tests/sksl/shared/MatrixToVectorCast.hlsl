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
  bool ok = false;
  float4 x_104 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_50 = false;
  bool x_51 = false;
  bool x_62 = false;
  bool x_63 = false;
  bool x_84 = false;
  bool x_85 = false;
  bool x_102 = false;
  bool x_103 = false;
  ok = true;
  x_51 = false;
  if (true) {
    const float2x2 x_37 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    x_50 = all((float4(x_37[0u].x, x_37[0u].y, x_37[1u].x, x_37[1u].y) == float4(1.0f, 2.0f, 3.0f, 4.0f)));
    x_51 = x_50;
  }
  ok = x_51;
  x_63 = false;
  if (x_51) {
    const float2x2 x_55 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    x_62 = all((float4(x_55[0u].x, x_55[0u].y, x_55[1u].x, x_55[1u].y) == float4(1.0f, 2.0f, 3.0f, 4.0f)));
    x_63 = x_62;
  }
  ok = x_63;
  x_85 = false;
  if (x_63) {
    const float2x2 x_67 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    const float x_68 = x_67[0u].x;
    const float x_69 = x_67[0u].y;
    const float x_70 = x_67[1u].x;
    const float x_71 = x_67[1u].y;
    const float4 x_72 = float4(x_68, x_69, x_70, x_71);
    x_84 = all((int4(int(x_68), int(x_69), int(x_70), int(x_71)) == int4(1, 2, 3, 4)));
    x_85 = x_84;
  }
  ok = x_85;
  x_103 = false;
  if (x_85) {
    const float2x2 x_89 = arr_to_mat2x2_stride_16(tint_symbol_3(x_10, 32u));
    const float x_90 = x_89[0u].x;
    const float x_91 = x_89[0u].y;
    const float x_92 = x_89[1u].x;
    const float x_93 = x_89[1u].y;
    const float4 x_94 = float4(x_90, x_91, x_92, x_93);
    x_102 = all((bool4(!((x_90 == 0.0f)), !((x_91 == 0.0f)), !((x_92 == 0.0f)), !((x_93 == 0.0f))) == (true).xxxx));
    x_103 = x_102;
  }
  ok = x_103;
  if (x_103) {
    const float4 x_112 = asfloat(x_10[0]);
    x_104 = x_112;
  } else {
    const float4 x_114 = asfloat(x_10[1]);
    x_104 = x_114;
  }
  const float4 x_115 = x_104;
  return x_115;
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
