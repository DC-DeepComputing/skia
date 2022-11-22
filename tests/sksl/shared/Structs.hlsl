struct A {
  int x;
  int y;
};
struct strided_arr {
  float el;
};
struct B {
  float x;
  strided_arr y[2];
  A z;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static A a1 = (A)0;
static B b1 = (B)0;

void main_1() {
  a1.x = 0;
  b1.x = 0.0f;
  const int x_29 = a1.x;
  const float x_32 = b1.x;
  sk_FragColor.x = (float(x_29) + x_32);
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  main_1();
  const main_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
