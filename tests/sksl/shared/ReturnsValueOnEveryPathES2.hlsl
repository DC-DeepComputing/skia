static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_15 : register(b0, space0) {
  uint4 x_15[3];
};

bool return_on_both_sides_b() {
  const float x_34 = asfloat(x_15[2].x);
  if ((x_34 == 1.0f)) {
    return true;
  } else {
    return true;
  }
}

bool for_inside_body_b() {
  int x = 0;
  x = 0;
  while (true) {
    const int x_50 = x;
    if ((x_50 <= 10)) {
    } else {
      break;
    }
    return true;
    {
      const int x_54 = x;
      x = (x_54 + 1);
    }
  }
  return false;
}

bool after_for_body_b() {
  int x_1 = 0;
  x_1 = 0;
  while (true) {
    const int x_63 = x_1;
    if ((x_63 <= 10)) {
    } else {
      break;
    }
    {
      const int x_65 = x_1;
      x_1 = (x_65 + 1);
    }
  }
  return true;
}

bool for_with_double_sided_conditional_return_b() {
  int x_2 = 0;
  x_2 = 0;
  while (true) {
    const int x_74 = x_2;
    if ((x_74 <= 10)) {
    } else {
      break;
    }
    const float x_77 = asfloat(x_15[2].x);
    if ((x_77 == 1.0f)) {
      return true;
    } else {
      return true;
    }
    {
      const int x_82 = x_2;
      x_2 = (x_82 + 1);
    }
  }
  return false;
}

bool if_else_chain_b() {
  const float x_86 = asfloat(x_15[2].x);
  if ((x_86 == 1.0f)) {
    return true;
  } else {
    const float x_92 = asfloat(x_15[2].x);
    if ((x_92 == 2.0f)) {
      return false;
    } else {
      const float x_100 = asfloat(x_15[2].x);
      if ((x_100 == 3.0f)) {
        return true;
      } else {
        const float x_107 = asfloat(x_15[2].x);
        if ((x_107 == 4.0f)) {
          return false;
        } else {
          return true;
        }
      }
    }
  }
}

float4 main(inout float2 x_114) {
  float4 x_136 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_118 = false;
  bool x_119 = false;
  bool x_122 = false;
  bool x_123 = false;
  bool x_126 = false;
  bool x_127 = false;
  bool x_130 = false;
  bool x_131 = false;
  bool x_134 = false;
  bool x_135 = false;
  x_119 = false;
  if (true) {
    x_118 = return_on_both_sides_b();
    x_119 = x_118;
  }
  x_123 = false;
  if (x_119) {
    x_122 = for_inside_body_b();
    x_123 = x_122;
  }
  x_127 = false;
  if (x_123) {
    x_126 = after_for_body_b();
    x_127 = x_126;
  }
  x_131 = false;
  if (x_127) {
    x_130 = for_with_double_sided_conditional_return_b();
    x_131 = x_130;
  }
  x_135 = false;
  if (x_131) {
    x_134 = if_else_chain_b();
    x_135 = x_134;
  }
  if (x_135) {
    const float4 x_143 = asfloat(x_15[0]);
    x_136 = x_143;
  } else {
    const float4 x_145 = asfloat(x_15[1]);
    x_136 = x_145;
  }
  const float4 x_146 = x_136;
  return x_146;
}

void _entrypoint_1() {
  float2 x_25 = float2(0.0f, 0.0f);
  x_25 = (0.0f).xx;
  const float4 x_27 = main(x_25);
  sk_FragColor = x_27;
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
