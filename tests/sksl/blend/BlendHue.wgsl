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
fn blend_color_saturation_Qhh3(color: vec3<f32>) -> f32 {
  {
    let _skTemp0 = max(color.x, color.y);
    let _skTemp1 = max(_skTemp0, color.z);
    let _skTemp2 = min(color.x, color.y);
    let _skTemp3 = min(_skTemp2, color.z);
    return _skTemp1 - _skTemp3;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _0_alpha: f32 = _globalUniforms.dst.w * _globalUniforms.src.w;
    let _1_sda: vec3<f32> = _globalUniforms.src.xyz * _globalUniforms.dst.w;
    let _2_dsa: vec3<f32> = _globalUniforms.dst.xyz * _globalUniforms.src.w;
    var _3_l: vec3<f32> = _1_sda;
    var _4_r: vec3<f32> = _2_dsa;
    {
      let _skTemp4 = min(_3_l.x, _3_l.y);
      let _skTemp5 = min(_skTemp4, _3_l.z);
      let _5_mn: f32 = _skTemp5;
      let _skTemp6 = max(_3_l.x, _3_l.y);
      let _skTemp7 = max(_skTemp6, _3_l.z);
      let _6_mx: f32 = _skTemp7;
      var _skTemp8: vec3<f32>;
      if _6_mx > _5_mn {
        let _skTemp9 = blend_color_saturation_Qhh3(_4_r);
        _skTemp8 = ((_3_l - _5_mn) * _skTemp9) / (_6_mx - _5_mn);
      } else {
        _skTemp8 = vec3<f32>(0.0);
      }
      _3_l = _skTemp8;
      _4_r = _2_dsa;
    }
    let _skTemp10 = dot(vec3<f32>(0.3, 0.59, 0.11), _4_r);
    let _7_lum: f32 = _skTemp10;
    let _skTemp11 = dot(vec3<f32>(0.3, 0.59, 0.11), _3_l);
    var _8_result: vec3<f32> = (_7_lum - _skTemp11) + _3_l;
    let _skTemp12 = min(_8_result.x, _8_result.y);
    let _skTemp13 = min(_skTemp12, _8_result.z);
    let _9_minComp: f32 = _skTemp13;
    let _skTemp14 = max(_8_result.x, _8_result.y);
    let _skTemp15 = max(_skTemp14, _8_result.z);
    let _10_maxComp: f32 = _skTemp15;
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
