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
fn guarded_divide_Qhhh(n: f32, d: f32) -> f32 {
  {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _0_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.x);
    var _skTemp0: f32;
    let _skTemp1 = abs(_globalUniforms.src.x);
    if _skTemp1 >= sk_PrivkMinNormalHalf {
      let _skTemp2 = guarded_divide_Qhhh((_globalUniforms.dst.w - _globalUniforms.dst.x) * _globalUniforms.src.w, _globalUniforms.src.x);
      let _skTemp3 = min(_globalUniforms.dst.w, _skTemp2);
      _skTemp0 = _globalUniforms.dst.w - _skTemp3;
    } else {
      _skTemp0 = _0_dyTerm;
    }
    let _1_delta: f32 = _skTemp0;
    let _2_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.y);
    var _skTemp4: f32;
    let _skTemp5 = abs(_globalUniforms.src.y);
    if _skTemp5 >= sk_PrivkMinNormalHalf {
      let _skTemp6 = guarded_divide_Qhhh((_globalUniforms.dst.w - _globalUniforms.dst.y) * _globalUniforms.src.w, _globalUniforms.src.y);
      let _skTemp7 = min(_globalUniforms.dst.w, _skTemp6);
      _skTemp4 = _globalUniforms.dst.w - _skTemp7;
    } else {
      _skTemp4 = _2_dyTerm;
    }
    let _3_delta: f32 = _skTemp4;
    let _4_dyTerm: f32 = select(0.0, _globalUniforms.dst.w, _globalUniforms.dst.w == _globalUniforms.dst.z);
    var _skTemp8: f32;
    let _skTemp9 = abs(_globalUniforms.src.z);
    if _skTemp9 >= sk_PrivkMinNormalHalf {
      let _skTemp10 = guarded_divide_Qhhh((_globalUniforms.dst.w - _globalUniforms.dst.z) * _globalUniforms.src.w, _globalUniforms.src.z);
      let _skTemp11 = min(_globalUniforms.dst.w, _skTemp10);
      _skTemp8 = _globalUniforms.dst.w - _skTemp11;
    } else {
      _skTemp8 = _4_dyTerm;
    }
    let _5_delta: f32 = _skTemp8;
    (*_stageOut).sk_FragColor = vec4<f32>((_1_delta * _globalUniforms.src.w + _globalUniforms.src.x * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.x * (1.0 - _globalUniforms.src.w), (_3_delta * _globalUniforms.src.w + _globalUniforms.src.y * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.y * (1.0 - _globalUniforms.src.w), (_5_delta * _globalUniforms.src.w + _globalUniforms.src.z * (1.0 - _globalUniforms.dst.w)) + _globalUniforms.dst.z * (1.0 - _globalUniforms.src.w), _globalUniforms.src.w + (1.0 - _globalUniforms.src.w) * _globalUniforms.dst.w);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
