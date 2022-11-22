float2 tint_unpack2x16float(uint param_0) {
  uint i = param_0;
  return f16tof32(uint2(i & 0xffff, i >> 16));
}

float2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}

float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

void main_1() {
  const uint x_22 = x_10[0].x;
  const float2 x_17 = tint_unpack2x16float(x_22);
  const float4 x_24 = sk_FragColor;
  sk_FragColor = float4(x_17.x, x_17.y, x_24.z, x_24.w);
  const uint x_28 = x_10[0].x;
  const float2 x_26 = tint_unpack2x16unorm(x_28);
  const float4 x_29 = sk_FragColor;
  sk_FragColor = float4(x_26.x, x_26.y, x_29.z, x_29.w);
  const uint x_33 = x_10[0].x;
  const float2 x_31 = tint_unpack2x16snorm(x_33);
  const float4 x_34 = sk_FragColor;
  sk_FragColor = float4(x_31.x, x_31.y, x_34.z, x_34.w);
  const uint x_38 = x_10[0].x;
  sk_FragColor = tint_unpack4x8unorm(x_38);
  const uint x_41 = x_10[0].x;
  sk_FragColor = tint_unpack4x8snorm(x_41);
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
  const main_out tint_symbol_4 = {sk_FragColor};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
