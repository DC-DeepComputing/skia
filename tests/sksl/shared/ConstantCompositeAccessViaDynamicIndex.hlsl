struct strided_arr {
  float el;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int zero = 0;
static strided_arr globalArray[2] = (strided_arr[2])0;
static float2x2 globalMatrix = float2x2(0.0f, 0.0f, 0.0f, 0.0f);

float4 main(inout float2 x_36) {
  strided_arr localArray[2] = (strided_arr[2])0;
  float2x2 localMatrix = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  zero = 0;
  const strided_arr tint_symbol_3 = {1.0f};
  const strided_arr tint_symbol_4 = {1.0f};
  const strided_arr tint_symbol_5[2] = {tint_symbol_3, tint_symbol_4};
  globalArray = tint_symbol_5;
  globalMatrix = float2x2((1.0f).xx, (1.0f).xx);
  const strided_arr tint_symbol_6 = (strided_arr)0;
  const strided_arr tint_symbol_7 = {1.0f};
  const strided_arr tint_symbol_8[2] = {tint_symbol_6, tint_symbol_7};
  localArray = tint_symbol_8;
  localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
  const int x_48 = zero;
  const float x_51 = globalArray[x_48].el;
  const int x_52 = zero;
  const float x_55 = localArray[x_52].el;
  const int x_57 = zero;
  const int x_59 = zero;
  const int x_62 = zero;
  const float2 x_65 = globalMatrix[x_62];
  const int x_66 = zero;
  const float2 x_68 = localMatrix[x_66];
  const float2 x_69 = (x_65 * x_68);
  return float4((x_51 * x_55), ((1.0f).xx[x_57] * (1.0f).xx[x_59]), x_69.x, x_69.y);
}

void _entrypoint_1() {
  float2 x_32 = float2(0.0f, 0.0f);
  x_32 = (0.0f).xx;
  const float4 x_34 = main(x_32);
  sk_FragColor = x_34;
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
  const _entrypoint_out tint_symbol_9 = {sk_FragColor};
  return tint_symbol_9;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
