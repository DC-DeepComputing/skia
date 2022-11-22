static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_14 : register(b0, space0) {
  uint4 x_14[1];
};

void outParameterWrite_vh4(inout float4 x_29) {
  const float4 x_35 = asfloat(x_14[0]);
  x_29 = x_35;
  return;
}

void outParameterWriteIndirect_vh4(inout float4 x_36) {
  float4 x_38 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  outParameterWrite_vh4(x_38);
  const float4 x_40 = x_38;
  x_36 = x_40;
  return;
}

void inoutParameterWrite_vh4(inout float4 x_41) {
  const float4 x_43 = x_41;
  const float4 x_44 = x_41;
  x_41 = (x_43 * x_44);
  return;
}

void inoutParameterWriteIndirect_vh4(inout float4 x_46) {
  float4 x_49 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_48 = x_46;
  x_49 = x_48;
  inoutParameterWrite_vh4(x_49);
  const float4 x_51 = x_49;
  x_46 = x_51;
  return;
}

float4 main(inout float2 x_53) {
  float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_56 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_59 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_62 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_65 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  outParameterWrite_vh4(x_56);
  const float4 x_58 = x_56;
  c = x_58;
  outParameterWriteIndirect_vh4(x_59);
  const float4 x_61 = x_59;
  c = x_61;
  x_62 = x_61;
  inoutParameterWrite_vh4(x_62);
  const float4 x_64 = x_62;
  c = x_64;
  x_65 = x_64;
  inoutParameterWriteIndirect_vh4(x_65);
  const float4 x_67 = x_65;
  c = x_67;
  return x_67;
}

void _entrypoint_1() {
  float2 x_24 = float2(0.0f, 0.0f);
  x_24 = (0.0f).xx;
  const float4 x_26 = main(x_24);
  sk_FragColor = x_26;
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
  const _entrypoint_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
