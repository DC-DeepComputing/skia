static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float4 live_fn_h4h4h4(inout float4 x_46, inout float4 x_47) {
  const float4 x_49 = x_46;
  const float4 x_50 = x_47;
  return (x_49 + x_50);
}

float4 unpremul_h4h4(inout float4 x_27) {
  const float4 x_29 = x_27;
  const float4 x_33 = x_27;
  const float3 x_38 = (float3(x_29.x, x_29.y, x_29.z) * (1.0f / max(x_33.w, 0.0001f)));
  const float4 x_42 = x_27;
  return float4(x_38.x, x_38.y, x_38.z, x_42.w);
}

float4 main(inout float2 x_53) {
  float4 a = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_59 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_62 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_65 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_77 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_75 = false;
  bool x_76 = false;
  x_59 = (3.0f).xxxx;
  x_62 = (-5.0f).xxxx;
  const float4 x_63 = live_fn_h4h4h4(x_59, x_62);
  a = x_63;
  x_65 = (1.0f).xxxx;
  const float4 x_66 = unpremul_h4h4(x_65);
  b = x_66;
  x_76 = false;
  if (any(!((x_63 == (0.0f).xxxx)))) {
    x_75 = any(!((x_66 == (0.0f).xxxx)));
    x_76 = x_75;
  }
  if (x_76) {
    const float4 x_85 = asfloat(x_12[0]);
    x_77 = x_85;
  } else {
    const float4 x_88 = asfloat(x_12[1]);
    x_77 = x_88;
  }
  const float4 x_89 = x_77;
  return x_89;
}

void _entrypoint_1() {
  float2 x_22 = float2(0.0f, 0.0f);
  x_22 = (0.0f).xx;
  const float4 x_24 = main(x_22);
  sk_FragColor = x_24;
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
