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
  testMatrix2x2: mat2x2<f32>,
  testMatrix3x3: mat3x3<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _0_ok: bool = true;
    _0_ok = _0_ok && (all(_globalUniforms.testMatrix2x2[0] == mat2x2<f32>(1.0, 2.0, 3.0, 4.0)[0]) && all(_globalUniforms.testMatrix2x2[1] == mat2x2<f32>(1.0, 2.0, 3.0, 4.0)[1]));
    let _skTemp0 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _0_ok = _0_ok && (all(_globalUniforms.testMatrix3x3[0] == _skTemp0[0]) && all(_globalUniforms.testMatrix3x3[1] == _skTemp0[1]) && all(_globalUniforms.testMatrix3x3[2] == _skTemp0[2]));
    _0_ok = _0_ok && (any(_globalUniforms.testMatrix2x2[0] != mat2x2<f32>(100.0, 0.0, 0.0, 100.0)[0]) || any(_globalUniforms.testMatrix2x2[1] != mat2x2<f32>(100.0, 0.0, 0.0, 100.0)[1]));
    let _skTemp1 = mat3x3<f32>(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    _0_ok = _0_ok && (any(_globalUniforms.testMatrix3x3[0] != _skTemp1[0]) || any(_globalUniforms.testMatrix3x3[1] != _skTemp1[1]) || any(_globalUniforms.testMatrix3x3[2] != _skTemp1[2]));
    var _1_zero: f32 = f32(_globalUniforms.colorGreen.x);
    var _2_one: f32 = f32(_globalUniforms.colorGreen.y);
    var _3_two: f32 = 2.0 * _2_one;
    var _4_nine: f32 = 9.0 * _2_one;
    let _skTemp2 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    _0_ok = _0_ok && (all(_skTemp2[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp2[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp3 = vec2<f32>(_2_one);
    let _skTemp4 = mat2x2<f32>(_2_one, _1_zero, _skTemp3[0], _skTemp3[1]);
    _0_ok = _0_ok && !(all(_skTemp4[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp4[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp5 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    _0_ok = _0_ok && (all(_skTemp5[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp5[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp6 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    _0_ok = _0_ok && !(all(_skTemp6[0] == mat2x2<f32>(0.0, 0.0, 0.0, 0.0)[0]) && all(_skTemp6[1] == mat2x2<f32>(0.0, 0.0, 0.0, 0.0)[1]));
    let _skTemp7 = -_2_one;
    let _skTemp8 = mat2x2<f32>(_skTemp7, 0.0, 0.0, _skTemp7);
    _0_ok = _0_ok && (all(_skTemp8[0] == mat2x2<f32>(-1.0, 0.0, 0.0, -1.0)[0]) && all(_skTemp8[1] == mat2x2<f32>(-1.0, 0.0, 0.0, -1.0)[1]));
    let _skTemp9 = mat2x2<f32>(_1_zero, 0.0, 0.0, _1_zero);
    _0_ok = _0_ok && (all(_skTemp9[0] == mat2x2<f32>(-0.0, 0.0, 0.0, -0.0)[0]) && all(_skTemp9[1] == mat2x2<f32>(-0.0, 0.0, 0.0, -0.0)[1]));
    let _skTemp10 = -_2_one;
    let _skTemp11 = (-1.0 * mat2x2<f32>(_skTemp10, 0.0, 0.0, _skTemp10));
    _0_ok = _0_ok && (all(_skTemp11[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp11[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp12 = (-1.0 * mat2x2<f32>(_1_zero, 0.0, 0.0, _1_zero));
    _0_ok = _0_ok && (all(_skTemp12[0] == mat2x2<f32>(-0.0, 0.0, 0.0, -0.0)[0]) && all(_skTemp12[1] == mat2x2<f32>(-0.0, 0.0, 0.0, -0.0)[1]));
    let _skTemp13 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    _0_ok = _0_ok && (all(_skTemp13[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp13[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp14 = mat2x2<f32>(_3_two, 0.0, 0.0, _3_two);
    _0_ok = _0_ok && !(all(_skTemp14[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp14[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp15 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    _0_ok = _0_ok && !(any(_skTemp15[0] != mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) || any(_skTemp15[1] != mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp16 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    _0_ok = _0_ok && (any(_skTemp16[0] != mat2x2<f32>(0.0, 0.0, 0.0, 0.0)[0]) || any(_skTemp16[1] != mat2x2<f32>(0.0, 0.0, 0.0, 0.0)[1]));
    let _skTemp17 = mat3x3<f32>(_2_one, _1_zero, _1_zero, _1_zero, _2_one, _1_zero, _1_zero, _1_zero, _2_one);
    let _skTemp18 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp19 = mat3x3<f32>(_skTemp18[0][0], _skTemp18[0][1], 0.0, _skTemp18[1][0], _skTemp18[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp17[0] == _skTemp19[0]) && all(_skTemp17[1] == _skTemp19[1]) && all(_skTemp17[2] == _skTemp19[2]));
    let _skTemp20 = mat3x3<f32>(_4_nine, _1_zero, _1_zero, _1_zero, _4_nine, _1_zero, _1_zero, _1_zero, _2_one);
    let _skTemp21 = mat2x2<f32>(9.0, 0.0, 0.0, 9.0);
    let _skTemp22 = mat3x3<f32>(_skTemp21[0][0], _skTemp21[0][1], 0.0, _skTemp21[1][0], _skTemp21[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp20[0] == _skTemp22[0]) && all(_skTemp20[1] == _skTemp22[1]) && all(_skTemp20[2] == _skTemp22[2]));
    let _skTemp23 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp24 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp25 = mat3x3<f32>(_skTemp24[0][0], _skTemp24[0][1], 0.0, _skTemp24[1][0], _skTemp24[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp23[0] == _skTemp25[0]) && all(_skTemp23[1] == _skTemp25[1]) && all(_skTemp23[2] == _skTemp25[2]));
    let _skTemp26 = mat3x3<f32>(_4_nine, 0.0, 0.0, 0.0, _4_nine, 0.0, 0.0, 0.0, _2_one);
    let _skTemp27 = mat2x2<f32>(9.0, 0.0, 0.0, 9.0);
    let _skTemp28 = mat3x3<f32>(_skTemp27[0][0], _skTemp27[0][1], 0.0, _skTemp27[1][0], _skTemp27[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp26[0] == _skTemp28[0]) && all(_skTemp26[1] == _skTemp28[1]) && all(_skTemp26[2] == _skTemp28[2]));
    let _skTemp29 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp30 = mat2x2<f32>(_skTemp29[0][0], _skTemp29[0][1], _skTemp29[1][0], _skTemp29[1][1]);
    _0_ok = _0_ok && (all(_skTemp30[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp30[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp31 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp32 = mat2x2<f32>(_skTemp31[0][0], _skTemp31[0][1], _skTemp31[1][0], _skTemp31[1][1]);
    _0_ok = _0_ok && (all(_skTemp32[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp32[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp33 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    _0_ok = _0_ok && (all(_skTemp33[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp33[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp34 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    _0_ok = _0_ok && (all(_skTemp34[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp34[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    let _skTemp35 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    _0_ok = _0_ok && (all(_skTemp35[0] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[0]) && all(_skTemp35[1] == mat2x2<f32>(1.0, 0.0, 0.0, 1.0)[1]));
    _0_ok = _0_ok && all(vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_2_one) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    _0_ok = _0_ok && all(vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_2_one) == vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])));
    _0_ok = _0_ok && all(vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_1_zero) == vec4<f32>(0.0));
    var _5_m: mat3x3<f32> = mat3x3<f32>(_2_one, _3_two, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, _4_nine);
    _0_ok = _0_ok && all(_5_m[0] == vec3<f32>(1.0, 2.0, 3.0));
    _0_ok = _0_ok && all(_5_m[1] == vec3<f32>(4.0, 5.0, 6.0));
    _0_ok = _0_ok && all(_5_m[2] == vec3<f32>(7.0, 8.0, 9.0));
    _0_ok = _0_ok && _5_m[0].x == 1.0;
    _0_ok = _0_ok && _5_m[0].y == 2.0;
    _0_ok = _0_ok && _5_m[0].z == 3.0;
    _0_ok = _0_ok && _5_m[1].x == 4.0;
    _0_ok = _0_ok && _5_m[1].y == 5.0;
    _0_ok = _0_ok && _5_m[1].z == 6.0;
    _0_ok = _0_ok && _5_m[2].x == 7.0;
    _0_ok = _0_ok && _5_m[2].y == 8.0;
    _0_ok = _0_ok && _5_m[2].z == 9.0;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_0_ok));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
