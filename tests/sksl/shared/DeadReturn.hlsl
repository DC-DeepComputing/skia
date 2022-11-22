static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int scratchVar = 0;
cbuffer cbuffer_x_18 : register(b0, space0) {
  uint4 x_18[2];
};

bool test_flat_b() {
  return true;
}

bool test_if_b() {
  const float4 x_37 = asfloat(x_18[0]);
  if ((x_37.y > 0.0f)) {
    return true;
  } else {
    const int x_44 = scratchVar;
    scratchVar = (x_44 + 1);
  }
  const int x_46 = scratchVar;
  scratchVar = (x_46 + 1);
  return false;
}

bool test_else_b() {
  const float4 x_51 = asfloat(x_18[0]);
  if ((x_51.y == 0.0f)) {
    return false;
  } else {
    return true;
  }
}

bool test_loop_if_b() {
  int x = 0;
  x = 0;
  while (true) {
    const int x_65 = x;
    if ((x_65 <= 1)) {
    } else {
      break;
    }
    const float4 x_68 = asfloat(x_18[0]);
    if ((x_68.y == 0.0f)) {
      return false;
    } else {
      return true;
    }
    {
      const int x_74 = x;
      x = (x_74 + 1);
    }
  }
  const int x_76 = scratchVar;
  scratchVar = (x_76 + 1);
  return true;
}

float4 main(inout float2 x_79) {
  float4 x_94 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_84 = false;
  bool x_85 = false;
  bool x_88 = false;
  bool x_89 = false;
  bool x_92 = false;
  bool x_93 = false;
  scratchVar = 0;
  const bool x_81 = test_flat_b();
  x_85 = false;
  if (x_81) {
    x_84 = test_if_b();
    x_85 = x_84;
  }
  x_89 = false;
  if (x_85) {
    x_88 = test_else_b();
    x_89 = x_88;
  }
  x_93 = false;
  if (x_89) {
    x_92 = test_loop_if_b();
    x_93 = x_92;
  }
  if (x_93) {
    const float4 x_100 = asfloat(x_18[0]);
    x_94 = x_100;
  } else {
    const float4 x_102 = asfloat(x_18[1]);
    x_94 = x_102;
  }
  const float4 x_103 = x_94;
  return x_103;
}

void _entrypoint_1() {
  float2 x_28 = float2(0.0f, 0.0f);
  x_28 = (0.0f).xx;
  const float4 x_30 = main(x_28);
  sk_FragColor = x_30;
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
