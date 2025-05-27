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
const sk_PrivkGuardedDivideEpsilon: f32 = f32(select(0.0, 1e-08, false));
const sk_PrivkMinNormalHalf: f32 = 6.10351562e-05;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _0_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.x);
    var _skTemp0: f32;
    let _skTemp1 = abs(_globalUniforms.src.x);
    if _skTemp1 >= sk_PrivkMinNormalHalf {
      let _skTemp2 = min(_globalUniforms.dst.w, ((_globalUniforms.dst.w - _globalUniforms.dst.x) * _globalUniforms.src.w) / (_globalUniforms.src.x + sk_PrivkGuardedDivideEpsilon));
      _skTemp0 = _globalUniforms.dst.w - _skTemp2;
    } else {
      _skTemp0 = _0_dyTerm;
    }
    let _1_delta: f32 = _skTemp0;
    let _2_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.y);
    var _skTemp3: f32;
    let _skTemp4 = abs(_globalUniforms.src.y);
    if _skTemp4 >= sk_PrivkMinNormalHalf {
      let _skTemp5 = min(_globalUniforms.dst.w, ((_globalUniforms.dst.w - _globalUniforms.dst.y) * _globalUniforms.src.w) / (_globalUniforms.src.y + sk_PrivkGuardedDivideEpsilon));
      _skTemp3 = _globalUniforms.dst.w - _skTemp5;
    } else {
      _skTemp3 = _2_dyTerm;
    }
    let _3_delta: f32 = _skTemp3;
    let _4_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.z);
    var _skTemp6: f32;
    let _skTemp7 = abs(_globalUniforms.src.z);
    if _skTemp7 >= sk_PrivkMinNormalHalf {
      let _skTemp8 = min(_globalUniforms.dst.w, ((_globalUniforms.dst.w - _globalUniforms.dst.z) * _globalUniforms.src.w) / (_globalUniforms.src.z + sk_PrivkGuardedDivideEpsilon));
      _skTemp6 = _globalUniforms.dst.w - _skTemp8;
    } else {
      _skTemp6 = _4_dyTerm;
    }
    let _5_delta: f32 = _skTemp6;
    (*_stageOut).sk_FragColor = vec4<f32>((_1_delta * _globalUniforms.src.w + _globalUniforms.src.x * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.x * (1.0 - _globalUniforms.src.w), (_3_delta * _globalUniforms.src.w + _globalUniforms.src.y * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.y * (1.0 - _globalUniforms.src.w), (_5_delta * _globalUniforms.src.w + _globalUniforms.src.z * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.z * (1.0 - _globalUniforms.src.w), _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
