static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 x_71 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_45 = false;
  bool x_46 = false;
  bool x_58 = false;
  bool x_59 = false;
  bool x_69 = false;
  bool x_70 = false;
  const float4 x_32 = asfloat(x_10[0]);
  x_46 = false;
  if ((round(x_32.x) == -1.0f)) {
    const float4 x_40 = asfloat(x_10[0]);
    x_45 = all((round(float2(x_40.x, x_40.y)) == float2(-1.0f, 0.0f)));
    x_46 = x_45;
  }
  x_59 = false;
  if (x_46) {
    const float4 x_51 = asfloat(x_10[0]);
    x_58 = all((round(float3(x_51.x, x_51.y, x_51.z)) == float3(-1.0f, 0.0f, 1.0f)));
    x_59 = x_58;
  }
  x_70 = false;
  if (x_59) {
    const float4 x_64 = asfloat(x_10[0]);
    x_69 = all((round(x_64) == float4(-1.0f, 0.0f, 1.0f, 2.0f)));
    x_70 = x_69;
  }
  if (x_70) {
    const float4 x_78 = asfloat(x_10[1]);
    x_71 = x_78;
  } else {
    const float4 x_81 = asfloat(x_10[2]);
    x_71 = x_81;
  }
  const float4 x_82 = x_71;
  return x_82;
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
