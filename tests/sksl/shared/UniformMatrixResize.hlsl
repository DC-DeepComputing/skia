static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[5];
};

float3x3 tint_symbol_3(uint4 buffer[5], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz));
}

float2x2 resizeMatrix_f22() {
  const float3x3 x_33 = tint_symbol_3(x_11, 0u);
  const float3 x_35 = x_33[0u];
  const float3 x_37 = x_33[1u];
  return float2x2(float2(x_35.x, x_35.y), float2(x_37.x, x_37.y));
}

float4 main(inout float2 x_41) {
  float4 x_58 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float2x2 x_43 = resizeMatrix_f22();
  if ((all((x_43[0u] == float2(1.0f, 2.0f))) & all((x_43[1u] == float2(4.0f, 5.0f))))) {
    const float4 x_66 = asfloat(x_11[3]);
    x_58 = x_66;
  } else {
    const float4 x_69 = asfloat(x_11[4]);
    x_58 = x_69;
  }
  const float4 x_70 = x_58;
  return x_70;
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
  const _entrypoint_out tint_symbol_6 = {sk_FragColor};
  return tint_symbol_6;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
