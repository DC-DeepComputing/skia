diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  I: vec4<f32>,
  N: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
<<<<<<< PATCH SET (0e6c5a038a96a7413822f2771b055df1da3adb43 [wgsl] Stop forcing all function calls into scratch lets)
    let _skTemp0 = -2e+34;
    var expectedX: f32 = 9.968786e+08 - 2 * _skTemp0 * 9.968786e+08 * _skTemp0;
||||||| BASE      (779301cbf33c320e285698558a5e04707b2b129e [wgsl] Refactor assemblePartialSampleCall to control closing)
    let _skTemp0 = -2e+34;
    let _skTemp1 = 9.968786e+08 - 2 * _skTemp0 * 9.968786e+08 * _skTemp0;
    var expectedX: f32 = _skTemp1;
=======
    let _skTemp0 = 65504.0;
    let _skTemp1 = (_skTemp0 * 222.0) * 2.0;
    let _skTemp2 = _skTemp1 - 2 * -65504.0 * _skTemp1 * -65504.0;
    var expectedX: f32 = _skTemp2;
>>>>>>> BASE      (a60a0a62865c1b7dbf3aaa34eb536b5ba5c794fd [wgsl] Refactor assemblePartialSampleCall to control closing)
    expectedX = -49.0;
    const expectedXY: vec2<f32> = vec2<f32>(-169.0, 202.0);
    const expectedXYZ: vec3<f32> = vec3<f32>(-379.0, 454.0, -529.0);
    const expectedXYZW: vec4<f32> = vec4<f32>(-699.0, 838.0, -977.0, 1116.0);
<<<<<<< PATCH SET (0e6c5a038a96a7413822f2771b055df1da3adb43 [wgsl] Stop forcing all function calls into scratch lets)
    let _skTemp1 = _globalUniforms.I.x;
    let _skTemp2 = _globalUniforms.N.x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp1 - 2 * _skTemp2 * _skTemp1 * _skTemp2 == expectedX) && all(reflect(_globalUniforms.I.xy, _globalUniforms.N.xy) == expectedXY)) && all(reflect(_globalUniforms.I.xyz, _globalUniforms.N.xyz) == expectedXYZ)) && all(reflect(_globalUniforms.I, _globalUniforms.N) == expectedXYZW)) && (-49.0 == expectedX)) && all(vec2<f32>(-169.0, 202.0) == expectedXY)) && all(vec3<f32>(-379.0, 454.0, -529.0) == expectedXYZ)) && all(vec4<f32>(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW)));
||||||| BASE      (779301cbf33c320e285698558a5e04707b2b129e [wgsl] Refactor assemblePartialSampleCall to control closing)
    let _skTemp2 = _globalUniforms.I.x;
    let _skTemp3 = _globalUniforms.N.x;
    let _skTemp4 = _skTemp2 - 2 * _skTemp3 * _skTemp2 * _skTemp3;
    let _skTemp5 = reflect(_globalUniforms.I.xy, _globalUniforms.N.xy);
    let _skTemp6 = reflect(_globalUniforms.I.xyz, _globalUniforms.N.xyz);
    let _skTemp7 = reflect(_globalUniforms.I, _globalUniforms.N);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp4 == expectedX) && all(_skTemp5 == expectedXY)) && all(_skTemp6 == expectedXYZ)) && all(_skTemp7 == expectedXYZW)) && (-49.0 == expectedX)) && all(vec2<f32>(-169.0, 202.0) == expectedXY)) && all(vec3<f32>(-379.0, 454.0, -529.0) == expectedXYZ)) && all(vec4<f32>(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW)));
=======
    let _skTemp3 = _globalUniforms.I.x;
    let _skTemp4 = _globalUniforms.N.x;
    let _skTemp5 = _skTemp3 - 2 * _skTemp4 * _skTemp3 * _skTemp4;
    let _skTemp6 = reflect(_globalUniforms.I.xy, _globalUniforms.N.xy);
    let _skTemp7 = reflect(_globalUniforms.I.xyz, _globalUniforms.N.xyz);
    let _skTemp8 = reflect(_globalUniforms.I, _globalUniforms.N);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp5 == expectedX) && all(_skTemp6 == expectedXY)) && all(_skTemp7 == expectedXYZ)) && all(_skTemp8 == expectedXYZW)) && (-49.0 == expectedX)) && all(vec2<f32>(-169.0, 202.0) == expectedXY)) && all(vec3<f32>(-379.0, 454.0, -529.0) == expectedXYZ)) && all(vec4<f32>(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW)));
>>>>>>> BASE      (a60a0a62865c1b7dbf3aaa34eb536b5ba5c794fd [wgsl] Refactor assemblePartialSampleCall to control closing)
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
