diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn blend_overlay_h4h4h4(src: vec4<f32>, dst: vec4<f32>) -> vec4<f32> {
  {
    var _skTemp0: f32;
    if (2.0 * dst.x) <= dst.w {
      _skTemp0 = (2.0 * src.x) * dst.x;
    } else {
      _skTemp0 = src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
    }
    var _skTemp1: f32;
    if (2.0 * dst.y) <= dst.w {
      _skTemp1 = (2.0 * src.y) * dst.y;
    } else {
      _skTemp1 = src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
    }
    var _skTemp2: f32;
    if (2.0 * dst.z) <= dst.w {
      _skTemp2 = (2.0 * src.z) * dst.z;
    } else {
      _skTemp2 = src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
    }
    var result: vec4<f32> = vec4<f32>(_skTemp0, _skTemp1, _skTemp2, src.w + (1.0 - src.w) * dst.w);
    result = vec4<f32>((result.xyz + (dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w))), result.w);
    return result;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp3 = blend_overlay_h4h4h4(_globalUniforms.dst, _globalUniforms.src);
    (*_stageOut).sk_FragColor = _skTemp3;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
