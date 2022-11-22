static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

float4 main(inout float2 x_24) {
  int integerInput = 0;
  int x_34 = 0;
  const float4 x_32 = asfloat(x_10[0]);
  x_34 = int(x_32.y);
  integerInput = x_34;
  if ((x_34 == 0)) {
    return (2.140000105f).xxxx;
  } else {
    if ((x_34 == 1)) {
      const float4 x_47 = asfloat(x_10[0]);
      return x_47;
    } else {
      if ((x_34 == 2)) {
        return float4(1.0f, 0.200000003f, 2.140000105f, 1.0f);
      } else {
        const float4 x_58 = asfloat(x_10[0]);
        if ((3.140000105f < (x_58.x * 3.140000105f))) {
          return (3.140000105f).xxxx;
        } else {
          const float4 x_67 = asfloat(x_10[0]);
          if ((2.140000105f >= (x_67.x * 2.140000105f))) {
            return (0.0f).xxxx;
          } else {
            return float4(1.0f, 0.0f, 0.0f, 1.0f);
          }
        }
      }
    }
  }
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
