diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  N: vec4<f32>,
  I: vec4<f32>,
  NRef: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = 1e+30;
<<<<<<< PATCH SET (0e6c5a038a96a7413822f2771b055df1da3adb43 [wgsl] Stop forcing all function calls into scratch lets)
    let huge: f32 = f32(select(-1.0, 1.0, _skTemp0 * 1e+30 < 0));
    let _skTemp1 = vec2<f32>(1.0);
    let huge2: vec2<f32> = faceForward(_skTemp1, vec2<f32>(1e+30), vec2<f32>(1e+30));
    let _skTemp2 = vec3<f32>(1.0);
    let huge3: vec3<f32> = faceForward(_skTemp2, vec3<f32>(1e+30), vec3<f32>(1e+30));
    let _skTemp3 = vec4<f32>(1.0);
    let huge4: vec4<f32> = faceForward(_skTemp3, vec4<f32>(1e+30), vec4<f32>(1e+30));
||||||| BASE      (779301cbf33c320e285698558a5e04707b2b129e [wgsl] Refactor assemblePartialSampleCall to control closing)
    let _skTemp1 = select(-1.0, 1.0, _skTemp0 * 1e+30 < 0);
    let huge: f32 = f32(_skTemp1);
    let _skTemp2 = vec2<f32>(1.0);
    let _skTemp3 = faceForward(_skTemp2, vec2<f32>(1e+30), vec2<f32>(1e+30));
    let huge2: vec2<f32> = _skTemp3;
    let _skTemp4 = vec3<f32>(1.0);
    let _skTemp5 = faceForward(_skTemp4, vec3<f32>(1e+30), vec3<f32>(1e+30));
    let huge3: vec3<f32> = _skTemp5;
    let _skTemp6 = vec4<f32>(1.0);
    let _skTemp7 = faceForward(_skTemp6, vec4<f32>(1e+30), vec4<f32>(1e+30));
    let huge4: vec4<f32> = _skTemp7;
=======
    let _skTemp1 = select(-1.0, 1.0, _skTemp0 * 1e+30 < 0);
    let huge: f32 = _skTemp1;
    let _skTemp2 = vec2<f32>(1.0);
    let _skTemp3 = faceForward(_skTemp2, vec2<f32>(1e+30), vec2<f32>(1e+30));
    let huge2: vec2<f32> = _skTemp3;
    let _skTemp4 = vec3<f32>(1.0);
    let _skTemp5 = faceForward(_skTemp4, vec3<f32>(1e+30), vec3<f32>(1e+30));
    let huge3: vec3<f32> = _skTemp5;
    let _skTemp6 = vec4<f32>(1.0);
    let _skTemp7 = faceForward(_skTemp6, vec4<f32>(1e+30), vec4<f32>(1e+30));
    let huge4: vec4<f32> = _skTemp7;
>>>>>>> BASE      (a60a0a62865c1b7dbf3aaa34eb536b5ba5c794fd [wgsl] Refactor assemblePartialSampleCall to control closing)
    var expectedPos: vec4<f32> = vec4<f32>(vec4<f32>(huge) + huge2.xxxx);
    var expectedNeg: vec4<f32> = vec4<f32>(huge3.xxxx + huge4.xxxx);
    expectedPos = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    expectedNeg = vec4<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp4 = _globalUniforms.N.x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((select(-_skTemp4, _skTemp4, _globalUniforms.I.x * _globalUniforms.NRef.x < 0) == expectedNeg.x) && all(faceForward(_globalUniforms.N.xy, _globalUniforms.I.xy, _globalUniforms.NRef.xy) == expectedNeg.xy)) && all(faceForward(_globalUniforms.N.xyz, _globalUniforms.I.xyz, _globalUniforms.NRef.xyz) == expectedPos.xyz)) && all(faceForward(_globalUniforms.N, _globalUniforms.I, _globalUniforms.NRef) == expectedPos)) && (-1.0 == expectedNeg.x)) && all(vec2<f32>(-1.0, -2.0) == expectedNeg.xy)) && all(vec3<f32>(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(vec4<f32>(1.0, 2.0, 3.0, 4.0) == expectedPos)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
