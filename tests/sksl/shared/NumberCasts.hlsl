static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 main(inout float2 x_21) {
  bool3 B = bool3(false, false, false);
  float3 F = float3(0.0f, 0.0f, 0.0f);
  int3 I = int3(0, 0, 0);
  bool x_65 = false;
  bool x_66 = false;
  bool x_70 = false;
  bool x_71 = false;
  B.x = true;
  B.y = true;
  B.z = true;
  F.x = 1.230000019f;
  F.y = 0.0f;
  F.z = 1.0f;
  I.x = 1;
  I.y = 1;
  I.z = 1;
  const float3 x_51 = F;
  const float3 x_53 = F;
  const float3 x_56 = F;
  const float x_58 = ((x_51.x * x_53.y) * x_56.z);
  const bool3 x_60 = B;
  x_66 = false;
  if (x_60.x) {
    const bool3 x_64 = B;
    x_65 = x_64.y;
    x_66 = x_65;
  }
  x_71 = false;
  if (x_66) {
    const bool3 x_69 = B;
    x_70 = x_69.z;
    x_71 = x_70;
  }
  const int3 x_73 = I;
  const int3 x_75 = I;
  const int3 x_78 = I;
  return float4(x_58, (x_71 ? 1.0f : 0.0f), 0.0f, float(((x_73.x * x_75.y) * x_78.z)));
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
  const _entrypoint_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
