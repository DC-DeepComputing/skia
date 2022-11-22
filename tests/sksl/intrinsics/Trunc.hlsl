static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 x_70 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_45 = false;
  bool x_46 = false;
  bool x_57 = false;
  bool x_58 = false;
  bool x_68 = false;
  bool x_69 = false;
  const float4 x_32 = asfloat(x_10[0]);
  x_46 = false;
  if ((trunc(x_32.x) == -1.0f)) {
    const float4 x_40 = asfloat(x_10[0]);
    x_45 = all((trunc(float2(x_40.x, x_40.y)) == float2(-1.0f, 0.0f)));
    x_46 = x_45;
  }
  x_58 = false;
  if (x_46) {
    const float4 x_51 = asfloat(x_10[0]);
    x_57 = all((trunc(float3(x_51.x, x_51.y, x_51.z)) == float3(-1.0f, 0.0f, 0.0f)));
    x_58 = x_57;
  }
  x_69 = false;
  if (x_58) {
    const float4 x_63 = asfloat(x_10[0]);
    x_68 = all((trunc(x_63) == float4(-1.0f, 0.0f, 0.0f, 2.0f)));
    x_69 = x_68;
  }
  if (x_69) {
    const float4 x_77 = asfloat(x_10[1]);
    x_70 = x_77;
  } else {
    const float4 x_80 = asfloat(x_10[2]);
    x_70 = x_80;
  }
  const float4 x_81 = x_70;
  return x_81;
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
