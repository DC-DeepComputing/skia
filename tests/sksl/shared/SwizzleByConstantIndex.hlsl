static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  float4 x_0_v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_1_x = 0.0f;
  float x_2_y = 0.0f;
  float x_3_z = 0.0f;
  float x_4_w = 0.0f;
  float4 a = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_9_x = 0.0f;
  float x_10_y = 0.0f;
  float x_11_z = 0.0f;
  float x_12_w = 0.0f;
  float4 b = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_84 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_78 = false;
  bool x_79 = false;
  bool x_83 = false;
  const float4 x_32 = asfloat(x_10[0]);
  x_0_v = x_32;
  const float x_35 = x_32.x;
  x_1_x = x_35;
  const float x_37 = x_32.y;
  x_2_y = x_37;
  const float x_39 = x_32.z;
  x_3_z = x_39;
  const float x_41 = x_32.w;
  x_4_w = x_41;
  const float4 x_43 = float4(x_35, x_37, x_39, x_41);
  a = x_43;
  const float4 x_46 = asfloat(x_10[0]);
  const float x_47 = x_46.x;
  x_9_x = x_47;
  const float4 x_50 = asfloat(x_10[0]);
  const float x_51 = x_50.y;
  x_10_y = x_51;
  const float4 x_54 = asfloat(x_10[0]);
  const float x_55 = x_54.z;
  x_11_z = x_55;
  const float4 x_58 = asfloat(x_10[0]);
  const float x_59 = x_58.w;
  x_12_w = x_59;
  const float4 x_61 = float4(x_47, x_51, x_55, x_59);
  b = x_61;
  c = float4(0.0f, 1.0f, 2.0f, 3.0f);
  x_79 = false;
  if (all((x_43 == float4(-1.25f, 0.0f, 0.75f, 2.25f)))) {
    x_78 = all((x_61 == float4(-1.25f, 0.0f, 0.75f, 2.25f)));
    x_79 = x_78;
  }
  x_83 = false;
  if (x_79) {
    x_83 = true;
  }
  if (x_83) {
    const float4 x_90 = asfloat(x_10[1]);
    x_84 = x_90;
  } else {
    const float4 x_93 = asfloat(x_10[2]);
    x_84 = x_93;
  }
  const float4 x_94 = x_84;
  return x_94;
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
