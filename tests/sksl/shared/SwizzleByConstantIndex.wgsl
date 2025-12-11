diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testInputs: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let _0_v = _globalUniforms.testInputs;
    let _1_x = _0_v.x;
    let _2_y = _0_v.y;
    let _3_z = _0_v.z;
    let _4_w = _0_v.w;
    let a = vec4<f16>(_1_x, _2_y, _3_z, _4_w);
    let _9_x = _globalUniforms.testInputs.x;
    let _10_y = _globalUniforms.testInputs.y;
    let _11_z = _globalUniforms.testInputs.z;
    let _12_w = _globalUniforms.testInputs.w;
    let b = vec4<f16>(_9_x, _10_y, _11_z, _12_w);
    const c = vec4<f16>(0.0h, 1.0h, 2.0h, 3.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((all(a == vec4<f16>(-1.25h, 0.0h, 0.75h, 2.25h)) && all(b == vec4<f16>(-1.25h, 0.0h, 0.75h, 2.25h))) && all(c == vec4<f16>(0.0h, 1.0h, 2.0h, 3.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
