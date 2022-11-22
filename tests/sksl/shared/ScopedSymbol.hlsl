struct S_1 {
  int i;
};

static bool sk_Clockwise = false;
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int glob = 0;
cbuffer cbuffer_x_17 : register(b0, space0) {
  uint4 x_17[2];
};

bool block_variable_hides_global_variable_b() {
  const int x_32 = glob;
  return (x_32 == 2);
}

bool local_variable_hides_struct_b() {
  bool S = false;
  S = true;
  return true;
}

bool local_struct_variable_hides_struct_type_b() {
  S_1 S_2 = (S_1)0;
  const S_1 tint_symbol_4 = {1};
  S_2 = tint_symbol_4;
  const int x_48 = S_2.i;
  return (x_48 == 1);
}

bool local_variable_hides_global_variable_b() {
  int glob_1 = 0;
  glob_1 = 1;
  return true;
}

float4 main(inout float2 x_53) {
  float4 x_72 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_58 = false;
  bool x_59 = false;
  bool x_62 = false;
  bool x_63 = false;
  bool x_66 = false;
  bool x_67 = false;
  bool x_70 = false;
  bool x_71 = false;
  glob = 2;
  x_59 = false;
  if (true) {
    x_58 = block_variable_hides_global_variable_b();
    x_59 = x_58;
  }
  x_63 = false;
  if (x_59) {
    x_62 = local_variable_hides_struct_b();
    x_63 = x_62;
  }
  x_67 = false;
  if (x_63) {
    x_66 = local_struct_variable_hides_struct_type_b();
    x_67 = x_66;
  }
  x_71 = false;
  if (x_67) {
    x_70 = local_variable_hides_global_variable_b();
    x_71 = x_70;
  }
  if (x_71) {
    const float4 x_79 = asfloat(x_17[0]);
    x_72 = x_79;
  } else {
    const float4 x_81 = asfloat(x_17[1]);
    x_72 = x_81;
  }
  const float4 x_82 = x_72;
  return x_82;
}

void _entrypoint_1() {
  float2 x_27 = float2(0.0f, 0.0f);
  x_27 = (0.0f).xx;
  const float4 x_29 = main(x_27);
  sk_FragColor = x_29;
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
  const _entrypoint_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 _entrypoint(tint_symbol_1 tint_symbol) {
  const _entrypoint_out inner_result = _entrypoint_inner(tint_symbol.sk_Clockwise_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
