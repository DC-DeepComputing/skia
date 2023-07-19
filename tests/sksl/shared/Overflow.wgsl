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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    let _skTemp0 = 9.000001e+35;
    var huge: f32 = f32((((((((((_skTemp0 * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09);
    let _skTemp1 = 1073741824;
    var hugeI: i32 = i32((((((((((((((((((((_skTemp1 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp2 = 2147483648u;
    var hugeU: u32 = ((((((((((((((((((_skTemp2 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp3 = 16384;
    var hugeS: i32 = ((((((((((((((((_skTemp3 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    let _skTemp4 = 32768u;
    var hugeUS: u32 = (((((((((((((((_skTemp4 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp5 = -2147483648;
    var hugeNI: i32 = i32(((((((((((((((((((_skTemp5 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp6 = -32768;
    var hugeNS: i32 = (((((((((((((((_skTemp6 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const i4: vec4<i32> = vec4<i32>(2);
    var hugeIvec: vec4<i32> = ((((((((((((((vec4<i32>(1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const u4: vec4<u32> = vec4<u32>(2u);
    var hugeUvec: vec4<u32> = (((((((((((((vec4<u32>(2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    let _skTemp7 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    var hugeMxM: mat4x4<f32> = _skTemp7 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let _skTemp8 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    var hugeMxV: vec4<f32> = _skTemp8 * vec4<f32>(1e+20);
    let _skTemp9 = vec4<f32>(1e+20);
    var hugeVxM: vec4<f32> = _skTemp9 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let _skTemp10 = saturate(huge);
    let _skTemp11 = saturate(f32(hugeI));
    let _skTemp12 = saturate(f32(hugeU));
    let _skTemp13 = saturate(f32(hugeS));
    let _skTemp14 = saturate(f32(hugeUS));
    let _skTemp15 = saturate(f32(hugeNI));
    let _skTemp16 = saturate(f32(hugeNS));
    let _skTemp17 = saturate(vec4<f32>(hugeIvec));
    let _skTemp18 = saturate(vec4<f32>(hugeUvec));
    let _skTemp19 = saturate(vec4<f32>(hugeMxM[0]));
    let _skTemp20 = saturate(vec4<f32>(hugeMxV));
    let _skTemp21 = saturate(vec4<f32>(hugeVxM));
    return (((((((((((_globalUniforms.colorGreen * _skTemp10) * _skTemp11) * _skTemp12) * _skTemp13) * _skTemp14) * _skTemp15) * _skTemp16) * _skTemp17) * _skTemp18) * _skTemp19) * _skTemp20) * _skTemp21;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
