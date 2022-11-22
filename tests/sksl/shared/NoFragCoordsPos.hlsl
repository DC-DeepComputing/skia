static float4 pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_13 = pos;
  gl_Position = x_13;
  return;
}

struct main_out {
  float4 gl_Position;
};
struct tint_symbol_1 {
  float4 pos_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 gl_Position : SV_Position;
};

main_out main_inner(float4 pos_param) {
  pos = pos_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.pos_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
