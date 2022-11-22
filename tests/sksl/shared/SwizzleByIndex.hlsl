static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};

float4 main(inout float2 x_24) {
  float4 x_0_v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int4 x_1_i = int4(0, 0, 0, 0);
  float x_2_x = 0.0f;
  float x_3_y = 0.0f;
  float x_4_z = 0.0f;
  float x_5_w = 0.0f;
  float4 x_67 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_32 = asfloat(x_10[0]);
  x_0_v = x_32;
  const float4 x_38 = asfloat(x_10[1]);
  const int4 x_47 = int4(int(x_38.x), int(x_38.y), int(x_38.z), int(x_38.w));
  x_1_i = x_47;
  const float x_51 = x_32[x_47.x];
  x_2_x = x_51;
  const float x_54 = x_32[x_47.y];
  x_3_y = x_54;
  const float x_57 = x_32[x_47.z];
  x_4_z = x_57;
  const float x_60 = x_32[x_47.w];
  x_5_w = x_60;
  if (all((float4(x_51, x_54, x_57, x_60) == float4(-1.25f, -1.25f, -1.25f, 0.0f)))) {
    const float4 x_73 = asfloat(x_10[2]);
    x_67 = x_73;
  } else {
    const float4 x_76 = asfloat(x_10[3]);
    x_67 = x_76;
  }
  const float4 x_77 = x_67;
  return x_77;
}

void _entrypoint_1() {
  float2 x_20 = float2(0.0f, 0.0f);
  x_20 = (0.0f).xx;
  const float4 x_22 = main(x_20);
  sk_FragColor = x_22;
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
