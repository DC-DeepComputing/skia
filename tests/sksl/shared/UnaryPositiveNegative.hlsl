struct strided_arr {
  float2 el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_16 : register(b0, space0) {
  uint4 x_16[12];
};

bool test_iscalar_b() {
  int x = 0;
  const float4 x_41 = asfloat(x_16[0]);
  const int x_43 = int(x_41.x);
  x = x_43;
  const int x_44 = -(x_43);
  x = x_44;
  return (x_44 == -1);
}

bool test_fvec_b() {
  float2 x_1 = float2(0.0f, 0.0f);
  const float4 x_50 = asfloat(x_16[0]);
  const float2 x_51 = float2(x_50.x, x_50.y);
  x_1 = x_51;
  const float2 x_52 = -(x_51);
  x_1 = x_52;
  return all((x_52 == (-1.0f).xx));
}

bool test_ivec_b() {
  int2 x_2 = int2(0, 0);
  const float4 x_63 = asfloat(x_16[0]);
  const int x_65 = int(x_63.x);
  const int2 x_66 = int2(x_65, x_65);
  x_2 = x_66;
  const int2 x_67 = -(x_66);
  x_2 = x_67;
  return all((x_67 == (-1).xx));
}

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

strided_arr tint_symbol_5(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const strided_arr tint_symbol_10 = {asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy))};
  return tint_symbol_10;
}

typedef strided_arr tint_symbol_4_ret[2];
tint_symbol_4_ret tint_symbol_4(uint4 buffer[12], uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_5(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

bool test_mat2_b() {
  float2x2 negated = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_3 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  negated = float2x2(float2(-1.0f, -2.0f), float2(-3.0f, -4.0f));
  const float2x2 x_84 = arr_to_mat2x2_stride_16(tint_symbol_4(x_16, 48u));
  x_3 = x_84;
  const float2 x_86 = -(x_84[0u]);
  const float2 x_88 = -(x_84[1u]);
  x_3 = float2x2(x_86, x_88);
  return (all((x_86 == float2(-1.0f, -2.0f))) & all((x_88 == float2(-3.0f, -4.0f))));
}

float3x3 tint_symbol_7(uint4 buffer[12], uint offset) {
  const uint scalar_offset_1 = ((offset + 0u)) / 4;
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  const uint scalar_offset_3 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

bool test_mat3_b() {
  float3x3 negated_1 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 x_4 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  negated_1 = float3x3(float3(-1.0f, -2.0f, -3.0f), float3(-4.0f, -5.0f, -6.0f), float3(-7.0f, -8.0f, -9.0f));
  const float3x3 x_111 = tint_symbol_7(x_16, 80u);
  x_4 = x_111;
  const float3 x_113 = -(x_111[0u]);
  const float3 x_115 = -(x_111[1u]);
  const float3 x_117 = -(x_111[2u]);
  x_4 = float3x3(x_113, x_115, x_117);
  return ((all((x_113 == float3(-1.0f, -2.0f, -3.0f))) & all((x_115 == float3(-4.0f, -5.0f, -6.0f)))) & all((x_117 == float3(-7.0f, -8.0f, -9.0f))));
}

float4x4 tint_symbol_9(uint4 buffer[12], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 16u)) / 4;
  const uint scalar_offset_6 = ((offset + 32u)) / 4;
  const uint scalar_offset_7 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset_4 / 4]), asfloat(buffer[scalar_offset_5 / 4]), asfloat(buffer[scalar_offset_6 / 4]), asfloat(buffer[scalar_offset_7 / 4]));
}

bool test_mat4_b() {
  float4x4 negated_2 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 x_5 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  negated_2 = float4x4(float4(-1.0f, -2.0f, -3.0f, -4.0f), float4(-5.0f, -6.0f, -7.0f, -8.0f), float4(-9.0f, -10.0f, -11.0f, -12.0f), float4(-13.0f, -14.0f, -15.0f, -16.0f));
  const float4x4 x_147 = tint_symbol_9(x_16, 128u);
  x_5 = x_147;
  const float4 x_149 = -(x_147[0u]);
  const float4 x_151 = -(x_147[1u]);
  const float4 x_153 = -(x_147[2u]);
  const float4 x_155 = -(x_147[3u]);
  x_5 = float4x4(x_149, x_151, x_153, x_155);
  return (((all((x_149 == float4(-1.0f, -2.0f, -3.0f, -4.0f))) & all((x_151 == float4(-5.0f, -6.0f, -7.0f, -8.0f)))) & all((x_153 == float4(-9.0f, -10.0f, -11.0f, -12.0f)))) & all((x_155 == float4(-13.0f, -14.0f, -15.0f, -16.0f))));
}

float4 main(inout float2 x_170) {
  float x_0_x = 0.0f;
  float4 x_204 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_182 = false;
  bool x_183 = false;
  bool x_186 = false;
  bool x_187 = false;
  bool x_190 = false;
  bool x_191 = false;
  bool x_194 = false;
  bool x_195 = false;
  bool x_198 = false;
  bool x_199 = false;
  bool x_202 = false;
  bool x_203 = false;
  const float4 x_175 = asfloat(x_16[0]);
  const float x_176 = x_175.x;
  x_0_x = x_176;
  const float x_177 = -(x_176);
  x_0_x = x_177;
  x_183 = false;
  if ((x_177 == -1.0f)) {
    x_182 = test_iscalar_b();
    x_183 = x_182;
  }
  x_187 = false;
  if (x_183) {
    x_186 = test_fvec_b();
    x_187 = x_186;
  }
  x_191 = false;
  if (x_187) {
    x_190 = test_ivec_b();
    x_191 = x_190;
  }
  x_195 = false;
  if (x_191) {
    x_194 = test_mat2_b();
    x_195 = x_194;
  }
  x_199 = false;
  if (x_195) {
    x_198 = test_mat3_b();
    x_199 = x_198;
  }
  x_203 = false;
  if (x_199) {
    x_202 = test_mat4_b();
    x_203 = x_202;
  }
  if (x_203) {
    const float4 x_211 = asfloat(x_16[1]);
    x_204 = x_211;
  } else {
    const float4 x_214 = asfloat(x_16[2]);
    x_204 = x_214;
  }
  const float4 x_215 = x_204;
  return x_215;
}

void _entrypoint_1() {
  float2 x_30 = float2(0.0f, 0.0f);
  x_30 = (0.0f).xx;
  const float4 x_32 = main(x_30);
  sk_FragColor = x_32;
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
  const _entrypoint_out tint_symbol_11 = {sk_FragColor};
  return tint_symbol_11;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
