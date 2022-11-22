static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

float userfunc_ff(inout float x_23) {
  const float x_25 = x_23;
  return (x_25 + 1.0f);
}

float4 main(inout float2 x_29) {
  float b = 0.0f;
  float c = 0.0f;
  float x_40 = 0.0f;
  float x_43 = 0.0f;
  int x = 0;
  float d = 0.0f;
  b = 2.0f;
  c = 3.0f;
  b = 2.0f;
  b = 80.0f;
  b = -0.993888676f;
  x_40 = 80.0f;
  const float x_41 = userfunc_ff(x_40);
  x_43 = 80.0f;
  const float x_44 = userfunc_ff(x_43);
  b = x_44;
  const float x_45 = -0.989992499f;
  b = x_45;
  b = x_45;
  x = 0;
  while (true) {
    const int x_55 = x;
    if ((x_55 < 1)) {
    } else {
      break;
    }
    {
      const int x_58 = x;
      x = (x_58 + 1);
    }
  }
  const float x_61 = c;
  d = x_61;
  b = 3.0f;
  const float x_62 = (x_61 + 1.0f);
  d = x_62;
  return float4(0.0f, 1.0f, ((x_62 == 5.0f) ? 1.0f : 0.0f), ((x_62 == 4.0f) ? 1.0f : 0.0f));
}

void _entrypoint_1() {
  float2 x_18 = float2(0.0f, 0.0f);
  x_18 = (0.0f).xx;
  const float4 x_20 = main(x_18);
  sk_FragColor = x_20;
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
