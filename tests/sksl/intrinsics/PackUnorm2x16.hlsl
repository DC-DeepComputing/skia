uint tint_pack2x16unorm(float2 param_0) {
  uint2 i = uint2(round(clamp(param_0, 0.0, 1.0) * 65535.0));
  return (i.x | i.y << 16);
}

float2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};

float4 main(inout float2 x_24) {
  uint xy = 0u;
  uint zw = 0u;
  float4 x_60 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_51 = false;
  bool x_59 = false;
  const float4 x_34 = asfloat(x_10[2]);
  const uint x_29 = tint_pack2x16unorm(float2(x_34.x, x_34.y));
  xy = x_29;
  const float4 x_39 = asfloat(x_10[2]);
  const uint x_37 = tint_pack2x16unorm(float2(x_39.z, x_39.w));
  zw = x_37;
  x_59 = false;
  if (all((abs(tint_unpack2x16unorm(x_29)) < (0.015625f).xx))) {
    x_51 = all((abs((tint_unpack2x16unorm(x_37) - float2(0.75f, 1.0f))) < (0.015625f).xx));
    x_59 = x_51;
  }
  if (x_59) {
    const float4 x_67 = asfloat(x_10[0]);
    x_60 = x_67;
  } else {
    const float4 x_70 = asfloat(x_10[1]);
    x_60 = x_70;
  }
  const float4 x_71 = x_60;
  return x_71;
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
