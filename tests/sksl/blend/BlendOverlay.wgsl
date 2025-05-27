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
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var _skTemp0: f32;
    if (2.0 * _globalUniforms.dst.x) <= _globalUniforms.dst.w {
      _skTemp0 = (2.0 * _globalUniforms.src.x) * _globalUniforms.dst.x;
    } else {
      _skTemp0 = _globalUniforms.src.w * _globalUniforms.dst.w - (2.0 * (_globalUniforms.dst.w - _globalUniforms.dst.x)) * (_globalUniforms.src.w - _globalUniforms.src.x);
    }
    var _skTemp1: f32;
    if (2.0 * _globalUniforms.dst.y) <= _globalUniforms.dst.w {
      _skTemp1 = (2.0 * _globalUniforms.src.y) * _globalUniforms.dst.y;
    } else {
      _skTemp1 = _globalUniforms.src.w * _globalUniforms.dst.w - (2.0 * (_globalUniforms.dst.w - _globalUniforms.dst.y)) * (_globalUniforms.src.w - _globalUniforms.src.y);
    }
    var _skTemp2: f32;
    if (2.0 * _globalUniforms.dst.z) <= _globalUniforms.dst.w {
      _skTemp2 = (2.0 * _globalUniforms.src.z) * _globalUniforms.dst.z;
    } else {
      _skTemp2 = _globalUniforms.src.w * _globalUniforms.dst.w - (2.0 * (_globalUniforms.dst.w - _globalUniforms.dst.z)) * (_globalUniforms.src.w - _globalUniforms.src.z);
    }
    var _0_result: vec4<f32> = vec4<f32>(_skTemp0, _skTemp1, _skTemp2, _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
    _0_result = vec4<f32>((_0_result.xyz + (_globalUniforms.dst.xyz * (1.0 - _globalUniforms.src.w) + _globalUniforms.src.xyz * (1.0 - _globalUniforms.dst.w))), _0_result.w);
    (*_stageOut).sk_FragColor = _0_result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
