static int sk_InstanceID = 0;
static int sk_VertexID = 0;
static float2 vcoord_Stage0 = float2(0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

int tint_mod(int lhs, int rhs) {
  return (lhs % (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

int tint_div(int lhs, int rhs) {
  return (lhs / (((rhs == 0) | ((lhs == -2147483648) & (rhs == -1))) ? 1 : rhs));
}

void main_1() {
  int x = 0;
  int y = 0;
  int ileft = 0;
  int iright = 0;
  int itop = 0;
  int ibot = 0;
  float outset = 0.0f;
  float x_63 = 0.0f;
  float l = 0.0f;
  float r = 0.0f;
  float t = 0.0f;
  float b = 0.0f;
  float2 vertexpos = float2(0.0f, 0.0f);
  float x_92 = 0.0f;
  float x_104 = 0.0f;
  const int x_20 = sk_InstanceID;
  const int x_22 = tint_mod(x_20, 200);
  x = x_22;
  const int x_24 = sk_InstanceID;
  const int x_25 = tint_div(x_24, 200);
  y = x_25;
  const int x_27 = sk_InstanceID;
  const int x_31 = tint_mod((x_27 * 929), 17);
  ileft = x_31;
  const int x_35 = sk_InstanceID;
  const int tint_symbol_3 = (x_31 + 1);
  const int tint_symbol_4 = tint_mod((x_35 * 1637), (17 - x_31));
  const int x_40 = (tint_symbol_3 + tint_symbol_4);
  iright = x_40;
  const int x_42 = sk_InstanceID;
  const int x_45 = tint_mod((x_42 * 313), 17);
  itop = x_45;
  const int x_48 = sk_InstanceID;
  const int tint_symbol_5 = (x_45 + 1);
  const int tint_symbol_6 = tint_mod((x_48 * 1901), (17 - x_45));
  const int x_53 = (tint_symbol_5 + tint_symbol_6);
  ibot = x_53;
  outset = 0.03125f;
  const int tint_symbol_7 = tint_mod((x_22 + x_25), 2);
  if ((0 == tint_symbol_7)) {
    x_63 = -0.03125f;
  } else {
    x_63 = 0.03125f;
  }
  const float x_68 = x_63;
  outset = x_68;
  const float x_73 = ((float(x_31) * 0.0625f) - x_68);
  l = x_73;
  const float x_77 = ((float(x_40) * 0.0625f) + x_68);
  r = x_77;
  t = ((float(x_45) * 0.0625f) - x_68);
  b = ((float(x_53) * 0.0625f) + x_68);
  const float x_88 = float(x_22);
  const int x_89 = sk_VertexID;
  const int tint_symbol_8 = tint_mod(x_89, 2);
  if ((0 == tint_symbol_8)) {
    x_92 = x_73;
  } else {
    x_92 = x_77;
  }
  const float x_96 = x_92;
  vertexpos.x = (x_88 + x_96);
  const int x_99 = y;
  const float x_100 = float(x_99);
  const int x_101 = sk_VertexID;
  const int tint_symbol_9 = tint_div(x_101, 2);
  if ((0 == tint_symbol_9)) {
    const float x_108 = t;
    x_104 = x_108;
  } else {
    const float x_109 = b;
    x_104 = x_109;
  }
  const float x_110 = x_104;
  vertexpos.y = (x_100 + x_110);
  const int x_113 = sk_VertexID;
  const int tint_symbol_10 = tint_mod(x_113, 2);
  const int tint_symbol_11 = ((0 == tint_symbol_10) ? -1 : 1);
  const float tint_symbol_12 = float(tint_symbol_11);
  vcoord_Stage0.x = tint_symbol_12;
  const int x_121 = sk_VertexID;
  const int tint_symbol_13 = tint_div(x_121, 2);
  const int tint_symbol_14 = ((0 == tint_symbol_13) ? -1 : 1);
  const float tint_symbol_15 = float(tint_symbol_14);
  vcoord_Stage0.y = tint_symbol_15;
  const float2 x_127 = vertexpos;
  const float2 x_129 = vertexpos;
  gl_Position = float4(x_127.x, x_129.y, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 gl_Position;
  float2 vcoord_Stage0_1;
};
struct tint_symbol_1 {
  uint sk_VertexID_param : SV_VertexID;
  uint sk_InstanceID_param : SV_InstanceID;
};
struct tint_symbol_2 {
  noperspective float2 vcoord_Stage0_1 : TEXCOORD1;
  float4 gl_Position : SV_Position;
};

main_out main_inner(uint sk_InstanceID_param, uint sk_VertexID_param) {
  sk_InstanceID = asint(sk_InstanceID_param);
  sk_VertexID = asint(sk_VertexID_param);
  main_1();
  const main_out tint_symbol_16 = {gl_Position, vcoord_Stage0};
  return tint_symbol_16;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_InstanceID_param, tint_symbol.sk_VertexID_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  wrapper_result.vcoord_Stage0_1 = inner_result.vcoord_Stage0_1;
  return wrapper_result;
}
