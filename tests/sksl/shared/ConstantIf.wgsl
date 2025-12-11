diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var a = 0;
    var b = 0;
    var c = 0;
    const d = 0;
    a = 1;
    b = 2;
    c = 5;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((a == 1) && (b == 2)) && (c == 5)) && (d == 0)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
