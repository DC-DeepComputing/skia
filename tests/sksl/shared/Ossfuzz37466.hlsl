struct strided_arr {
  float el;
};

static bool sk_Clockwise = false;

void main_1() {
  strided_arr y[2] = (strided_arr[2])0;
  strided_arr x_0_v[2] = (strided_arr[2])0;
  const strided_arr x_16[2] = y;
  x_0_v = x_16;
  const float x_20 = x_0_v[1].el;
  x_0_v[0].el = x_20;
  return;
}

struct tint_symbol_1 {
  bool sk_Clockwise_param : SV_IsFrontFace;
};

void main_inner(bool sk_Clockwise_param) {
  sk_Clockwise = sk_Clockwise_param;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.sk_Clockwise_param);
  return;
}
