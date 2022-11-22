static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  float4 x = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float r = 0.0f;
  float b = 0.0f;
  const float4 x_32 = asfloat(x_10[0]);
  x = x_32;
  r = -5.0f;
  while (true) {
    const float x_41 = r;
    if ((x_41 < 5.0f)) {
    } else {
      break;
    }
    const float x_45 = r;
    x.x = clamp(x_45, 0.0f, 1.0f);
    const float4 x_48 = x;
    if ((x_48.x == 0.0f)) {
      break;
    }
    {
      const float x_53 = r;
      r = (x_53 + 1.0f);
    }
  }
  b = 5.0f;
  while (true) {
    const float x_61 = b;
    if ((x_61 >= 0.0f)) {
    } else {
      break;
    }
    const float x_63 = b;
    x.z = x_63;
    const float4 x_66 = x;
    if ((x_66.w == 1.0f)) {
      {
        const float x_73 = b;
        b = (x_73 - 1.0f);
      }
      continue;
    }
    x.y = 0.0f;
    {
      const float x_73 = b;
      b = (x_73 - 1.0f);
    }
  }
  const float4 x_75 = x;
  return x_75;
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
