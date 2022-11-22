static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 expected = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_78 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_47 = false;
  bool x_48 = false;
  bool x_56 = false;
  bool x_57 = false;
  bool x_63 = false;
  bool x_64 = false;
  bool x_68 = false;
  bool x_71 = false;
  bool x_74 = false;
  bool x_77 = false;
  expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
  const float4 x_38 = asfloat(x_10[0]);
  x_48 = false;
  if ((length(x_38.x) == 3.0f)) {
    const float4 x_45 = asfloat(x_10[0]);
    x_47 = (length(float2(x_45.x, x_45.y)) == 3.0f);
    x_48 = x_47;
  }
  x_57 = false;
  if (x_48) {
    const float4 x_53 = asfloat(x_10[0]);
    x_56 = (length(float3(x_53.x, x_53.y, x_53.z)) == 5.0f);
    x_57 = x_56;
  }
  x_64 = false;
  if (x_57) {
    const float4 x_62 = asfloat(x_10[0]);
    x_63 = (length(x_62) == 13.0f);
    x_64 = x_63;
  }
  x_68 = false;
  if (x_64) {
    x_68 = true;
  }
  x_71 = false;
  if (x_68) {
    x_71 = true;
  }
  x_74 = false;
  if (x_71) {
    x_74 = true;
  }
  x_77 = false;
  if (x_74) {
    x_77 = true;
  }
  if (x_77) {
    const float4 x_84 = asfloat(x_10[1]);
    x_78 = x_84;
  } else {
    const float4 x_87 = asfloat(x_10[2]);
    x_78 = x_87;
  }
  const float4 x_88 = x_78;
  return x_88;
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
