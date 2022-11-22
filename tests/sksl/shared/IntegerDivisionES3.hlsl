static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

float4 main(inout float2 x_24) {
  int zero = 0;
  int one = 0;
  int x = 0;
  int y = 0;
  int x_0_x = 0;
  int x_1_result = 0;
  const float4 x_32 = asfloat(x_10[0]);
  const int x_34 = int(x_32.x);
  zero = x_34;
  const float4 x_37 = asfloat(x_10[0]);
  one = int(x_37.y);
  x = x_34;
  while (true) {
    const int x_46 = x;
    if ((x_46 < 100)) {
    } else {
      break;
    }
    const int x_50 = one;
    y = x_50;
    while (true) {
      const int x_56 = y;
      if ((x_56 < 100)) {
      } else {
        break;
      }
      const int x_59 = x;
      x_0_x = x_59;
      x_1_result = 0;
      while (true) {
        const int x_66 = x_0_x;
        const int x_67 = y;
        if ((x_66 >= x_67)) {
        } else {
          break;
        }
        const int x_70 = x_1_result;
        x_1_result = (x_70 + 1);
        const int x_72 = x_0_x;
        const int x_73 = y;
        x_0_x = (x_72 - x_73);
      }
      const int x_75 = x;
      const int x_76 = y;
      const int x_78 = x_1_result;
      const int tint_symbol_3 = tint_div(x_75, x_76);
      if ((tint_symbol_3 != x_78)) {
        const int x_83 = x;
        const int x_87 = y;
        return float4(1.0f, (float(x_83) * 0.003921569f), (float(x_87) * 0.003921569f), 1.0f);
      }
      {
        const int x_91 = y;
        y = (x_91 + 1);
      }
    }
    {
      const int x_93 = x;
      x = (x_93 + 1);
    }
  }
  const float4 x_96 = asfloat(x_10[0]);
  return x_96;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
