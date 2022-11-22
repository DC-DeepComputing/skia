cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  gl_Position = (1.0f).xxxx;
  const float4 x_21 = gl_Position;
  const float4 x_26 = asfloat(x_8[0]);
  const float4 x_30 = gl_Position;
  const float4 x_33 = asfloat(x_8[0]);
  const float2 x_36 = ((float2(x_21.x, x_21.y) * float2(x_26.x, x_26.z)) + (float2(x_30.w, x_30.w) * float2(x_33.y, x_33.w)));
  const float4 x_41 = gl_Position;
  gl_Position = float4(x_36.x, x_36.y, 0.0f, x_41.w);
  return;
}

struct main_out {
  float4 gl_Position;
};
struct tint_symbol {
  float4 gl_Position : SV_Position;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {gl_Position};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
