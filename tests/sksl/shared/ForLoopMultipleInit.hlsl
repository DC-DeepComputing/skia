struct strided_arr {
  float el;
};
struct strided_arr_1 {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 main(inout float2 x_21) {
  float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float a = 0.0f;
  float b = 0.0f;
  int c = 0;
  strided_arr d[2] = (strided_arr[2])0;
  strided_arr_1 e[4] = (strided_arr_1[4])0;
  float f = 0.0f;
  result = (0.0f).xxxx;
  a = 0.0f;
  b = 0.0f;
  while (true) {
    bool x_41 = false;
    bool x_42 = false;
    const float x_35 = a;
    x_42 = false;
    if ((x_35 < 10.0f)) {
      const float x_40 = b;
      x_41 = (x_40 < 10.0f);
      x_42 = x_41;
    }
    if (x_42) {
    } else {
      break;
    }
    const float x_46 = result.x;
    const float x_47 = a;
    result.x = (x_46 + x_47);
    const float x_51 = result.y;
    const float x_52 = b;
    result.y = (x_51 + x_52);
    {
      const float x_55 = a;
      a = (x_55 + 1.0f);
      const float x_57 = b;
      b = (x_57 + 1.0f);
    }
  }
  c = 0;
  while (true) {
    const int x_66 = c;
    if ((x_66 < 10)) {
    } else {
      break;
    }
    const float x_71 = result.z;
    result.z = (x_71 + 1.0f);
    {
      const int x_73 = c;
      c = (x_73 + 1);
    }
  }
  const strided_arr tint_symbol_3 = (strided_arr)0;
  const strided_arr tint_symbol_4 = {10.0f};
  const strided_arr tint_symbol_5[2] = {tint_symbol_3, tint_symbol_4};
  d = tint_symbol_5;
  const strided_arr_1 tint_symbol_6 = {1.0f};
  const strided_arr_1 tint_symbol_7 = {2.0f};
  const strided_arr_1 tint_symbol_8 = {3.0f};
  const strided_arr_1 tint_symbol_9 = {4.0f};
  const strided_arr_1 tint_symbol_10[4] = {tint_symbol_6, tint_symbol_7, tint_symbol_8, tint_symbol_9};
  e = tint_symbol_10;
  f = 9.0f;
  while (true) {
    const float x_95 = d[0].el;
    const float x_97 = d[1].el;
    if ((x_95 < x_97)) {
    } else {
      break;
    }
    const float x_100 = e[0].el;
    const float x_101 = f;
    result.w = (x_100 * x_101);
    {
      const float x_106 = d[0].el;
      d[0].el = (x_106 + 1.0f);
    }
  }
  while (true) {
    break;
  }
  while (true) {
    break;
  }
  const float4 x_118 = result;
  return x_118;
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
  const _entrypoint_out tint_symbol_11 = {sk_FragColor};
  return tint_symbol_11;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
