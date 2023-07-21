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
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    let _skTemp0 = 1e+30;
    let _skTemp1 = (select(-1.0, 1.0, _skTemp0 * 1e+30 < 0) * 1.0);
    var huge: f32 = f32(_skTemp1);
    let _skTemp2 = vec2<f32>(1.0);
    let _skTemp3 = faceForward(_skTemp2, vec2<f32>(1e+30), vec2<f32>(1e+30));
    var huge2: vec2<f32> = _skTemp3;
    let _skTemp4 = vec3<f32>(1.0);
    let _skTemp5 = faceForward(_skTemp4, vec3<f32>(1e+30), vec3<f32>(1e+30));
    var huge3: vec3<f32> = _skTemp5;
    let _skTemp6 = vec4<f32>(1.0);
    let _skTemp7 = faceForward(_skTemp6, vec4<f32>(1e+30), vec4<f32>(1e+30));
    var huge4: vec4<f32> = _skTemp7;
    var expectedPos: vec4<f32> = vec4<f32>(vec4<f32>(huge) + huge2.xxxx);
    var expectedNeg: vec4<f32> = vec4<f32>(huge3.xxxx + huge4.xxxx);
    const constN: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    const constI: vec4<f32> = vec4<f32>(1.0, 1.0, -100.0, 1.0);
    var N: vec4<f32> = _globalUniforms.colorGreen.yyyy * constN;
    var I: vec4<f32> = _globalUniforms.colorGreen.yyyy * constI;
    var NRef: vec4<f32> = _globalUniforms.colorGreen.yyyy;
    expectedPos = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    expectedNeg = vec4<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp8 = (select(-1.0, 1.0, I.x * NRef.x < 0) * N.x);
    let _skTemp9 = faceForward(N.xy, I.xy, NRef.xy);
    let _skTemp10 = faceForward(N.xyz, I.xyz, NRef.xyz);
    let _skTemp11 = faceForward(N, I, NRef);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp8 == expectedNeg.x) && all(_skTemp9 == expectedNeg.xy)) && all(_skTemp10 == expectedPos.xyz)) && all(_skTemp11 == expectedPos)) && (-1.0 == expectedNeg.x)) && all(vec2<f32>(-1.0, -2.0) == expectedNeg.xy)) && all(vec3<f32>(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(vec4<f32>(1.0, 2.0, 3.0, 4.0) == expectedPos)));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
