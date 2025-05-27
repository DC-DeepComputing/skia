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
    let _0_alpha: f32 = _globalUniforms.dst.w * _globalUniforms.src.w;
    let _1_sda: vec3<f32> = _globalUniforms.src.xyz * _globalUniforms.dst.w;
    let _2_dsa: vec3<f32> = _globalUniforms.dst.xyz * _globalUniforms.src.w;
    let _3_l: vec3<f32> = _2_dsa;
    let _4_r: vec3<f32> = _1_sda;
    let _skTemp0 = dot(vec3<f32>(0.3, 0.59, 0.11), _4_r);
    let _5_lum: f32 = _skTemp0;
    let _skTemp1 = dot(vec3<f32>(0.3, 0.59, 0.11), _3_l);
    var _6_result: vec3<f32> = (_5_lum - _skTemp1) + _3_l;
    let _skTemp2 = min(_6_result.x, _6_result.y);
    let _skTemp3 = min(_skTemp2, _6_result.z);
    let _7_minComp: f32 = _skTemp3;
    let _skTemp4 = max(_6_result.x, _6_result.y);
    let _skTemp5 = max(_skTemp4, _6_result.z);
    let _8_maxComp: f32 = _skTemp5;
    if (_7_minComp < 0.0) && (_5_lum != _7_minComp) {
      {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
      }
    }
    if (_8_maxComp > _0_alpha) && (_8_maxComp != _5_lum) {
      {
        _6_result = _5_lum + ((_6_result - _5_lum) * (_0_alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
      }
    }
    (*_stageOut).sk_FragColor = vec4<f32>((((_6_result + _globalUniforms.dst.xyz) - _2_dsa) + _globalUniforms.src.xyz) - _1_sda, (_globalUniforms.src.w + _globalUniforms.dst.w) - _0_alpha);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
