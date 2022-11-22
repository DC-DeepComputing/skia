static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[7];
};

float4 main(inout float2 x_27) {
  int index = 0;
  index = 0;
  while (true) {
    const int x_37 = index;
    if ((x_37 < 5)) {
    } else {
      break;
    }
    const int x_41 = index;
    const uint scalar_offset = ((16u * uint(x_41))) / 4;
    const float x_44 = asfloat(x_10[scalar_offset / 4][scalar_offset % 4]);
    const int x_45 = index;
    if (!((x_44 == float((x_45 + 1))))) {
      const float4 x_55 = asfloat(x_10[6]);
      return x_55;
    }
    {
      const int x_56 = index;
      index = (x_56 + 1);
    }
  }
  const float4 x_59 = asfloat(x_10[5]);
  return x_59;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
