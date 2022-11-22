static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 main(inout float2 x_21) {
  float4 x = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x = (1.0f).xxxx;
  while (true) {
    const float4 x_32 = x;
    if ((x_32.w == 1.0f)) {
    } else {
      break;
    }
    const float x_39 = x.x;
    x.x = (x_39 - 0.25f);
    const float4 x_42 = x;
    if ((x_42.x <= 0.0f)) {
      break;
    }
  }
  while (true) {
    const float4 x_52 = x;
    if ((x_52.z > 0.0f)) {
    } else {
      break;
    }
    const float x_57 = x.z;
    x.z = (x_57 - 0.25f);
    const float4 x_59 = x;
    if ((x_59.w == 1.0f)) {
      continue;
    }
    x.y = 0.0f;
  }
  const float4 x_66 = x;
  return x_66;
}

void _entrypoint_1() {
  float2 x_17 = float2(0.0f, 0.0f);
  x_17 = (0.0f).xx;
  const float4 x_19 = main(x_17);
  sk_FragColor = x_19;
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
  const _entrypoint_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
