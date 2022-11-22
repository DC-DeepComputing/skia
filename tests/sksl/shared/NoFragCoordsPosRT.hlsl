static float4 pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_16 = pos;
  gl_Position = x_16;
  const float4 x_22 = gl_Position;
  const float4 x_27 = asfloat(x_10[0]);
  const float4 x_31 = gl_Position;
  const float4 x_34 = asfloat(x_10[0]);
  const float2 x_37 = ((float2(x_22.x, x_22.y) * float2(x_27.x, x_27.z)) + (float2(x_31.w, x_31.w) * float2(x_34.y, x_34.w)));
  const float4 x_42 = gl_Position;
  gl_Position = float4(x_37.x, x_37.y, 0.0f, x_42.w);
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
  const main_out tint_symbol_4 = {gl_Position};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.pos_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
