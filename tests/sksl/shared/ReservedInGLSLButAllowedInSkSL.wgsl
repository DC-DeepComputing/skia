diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let R_active = _globalUniforms.colorGreen;
    let R_centroid = _globalUniforms.colorGreen;
    let R_coherent = _globalUniforms.colorGreen;
    let R_common = _globalUniforms.colorGreen;
    let R_filter = _globalUniforms.colorGreen;
    let R_partition = _globalUniforms.colorGreen;
    let R_patch = _globalUniforms.colorGreen;
    let R_precise = _globalUniforms.colorGreen;
    let R_resource = _globalUniforms.colorGreen;
    let R_restrict = _globalUniforms.colorGreen;
    let R_shared = _globalUniforms.colorGreen;
    let R_smooth = _globalUniforms.colorGreen;
    let R_subroutine = _globalUniforms.colorGreen;
    return (((((((((((R_active * R_centroid) * R_coherent) * R_common) * R_filter) * R_partition) * R_patch) * R_precise) * R_resource) * R_restrict) * R_shared) * R_smooth) * R_subroutine;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
