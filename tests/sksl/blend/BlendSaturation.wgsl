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
    var _3_l: vec3<f32> = _2_dsa;
    var _4_r: vec3<f32> = _1_sda;
    {
      let _skTemp0 = min(_3_l.x, _3_l.y);
      let _skTemp1 = min(_skTemp0, _3_l.z);
      let _5_mn: f32 = _skTemp1;
      let _skTemp2 = max(_3_l.x, _3_l.y);
      let _skTemp3 = max(_skTemp2, _3_l.z);
      let _6_mx: f32 = _skTemp3;
      var _skTemp4: vec3<f32>;
      if _6_mx > _5_mn {
        let _skTemp5 = max(_4_r.x, _4_r.y);
        let _skTemp6 = max(_skTemp5, _4_r.z);
        let _skTemp7 = min(_4_r.x, _4_r.y);
        let _skTemp8 = min(_skTemp7, _4_r.z);
        _skTemp4 = ((_3_l - _5_mn) * (_skTemp6 - _skTemp8)) / (_6_mx - _5_mn);
      } else {
        _skTemp4 = vec3<f32>(0.0);
      }
      _3_l = _skTemp4;
      _4_r = _2_dsa;
    }
    let _skTemp9 = dot(vec3<f32>(0.3, 0.59, 0.11), _4_r);
    let _7_lum: f32 = _skTemp9;
    let _skTemp10 = dot(vec3<f32>(0.3, 0.59, 0.11), _3_l);
    var _8_result: vec3<f32> = (_7_lum - _skTemp10) + _3_l;
    let _skTemp11 = min(_8_result.x, _8_result.y);
    let _skTemp12 = min(_skTemp11, _8_result.z);
    let _9_minComp: f32 = _skTemp12;
    let _skTemp13 = max(_8_result.x, _8_result.y);
    let _skTemp14 = max(_skTemp13, _8_result.z);
    let _10_maxComp: f32 = _skTemp14;
    if (_9_minComp < 0.0) && (_7_lum != _9_minComp) {
      {
        _8_result = _7_lum + (_8_result - _7_lum) * (_7_lum / (((_7_lum - _9_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
      }
    }
    if (_10_maxComp > _0_alpha) && (_10_maxComp != _7_lum) {
      {
        _8_result = _7_lum + ((_8_result - _7_lum) * (_0_alpha - _7_lum)) / (((_10_maxComp - _7_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
      }
    }
    (*_stageOut).sk_FragColor = vec4<f32>((((_8_result + _globalUniforms.dst.xyz) - _2_dsa) + _globalUniforms.src.xyz) - _1_sda, (_globalUniforms.src.w + _globalUniforms.dst.w) - _0_alpha);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
