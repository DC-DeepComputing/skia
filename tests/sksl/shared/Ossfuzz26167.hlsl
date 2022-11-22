static bool sk_Clockwise = false;

void main_1() {
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
