diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat3_inverse(m: mat3x3<f32>) -> mat3x3<f32> {
let a00 = m[0].x; let a01 = m[0].y; let a02 = m[0].z;
let a10 = m[1].x; let a11 = m[1].y; let a12 = m[1].z;
let a20 = m[2].x; let a21 = m[2].y; let a22 = m[2].z;
let b01 =  a22*a11 - a12*a21;
let b11 = -a22*a10 + a12*a20;
let b21 =  a21*a10 - a11*a20;
let det = a00*b01 + a01*b11 + a02*b21;
return mat3x3<f32>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var inv2x2: mat2x2<f32> = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    var inv3x3: mat3x3<f32> = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    var inv4x4: mat4x4<f32> = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    let _skTemp0 = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    let _skTemp1 = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    let _skTemp2 = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    let _skTemp3 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    let _skTemp4 = mat3_inverse(_skTemp3);
    let _skTemp5 = _skTemp4;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((all(_skTemp0[0] == inv2x2[0]) && all(_skTemp0[1] == inv2x2[1])) && (all(_skTemp1[0] == inv3x3[0]) && all(_skTemp1[1] == inv3x3[1]) && all(_skTemp1[2] == inv3x3[2]))) && (all(_skTemp2[0] == inv4x4[0]) && all(_skTemp2[1] == inv4x4[1]) && all(_skTemp2[2] == inv4x4[2]) && all(_skTemp2[3] == inv4x4[3]))) && (any(_skTemp5[0] != inv3x3[0]) || any(_skTemp5[1] != inv3x3[1]) || any(_skTemp5[2] != inv3x3[2]))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
