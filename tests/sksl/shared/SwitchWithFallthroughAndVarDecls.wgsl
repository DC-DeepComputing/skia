struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var ok: bool;
    {
      var a: bool;
      var b: i32;
      var c: i32;
      let _skTemp0 = i32(_globalUniforms.colorGreen.y);
      switch _skTemp0 {
        case 1, 2, 3, 4 {
          var _skTemp1: bool = false;
          if _skTemp0 == 1 {
            b = 1;
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 2 {
            {
              var d: f32 = f32(b);
              c = i32(d);
            }
            _skTemp1 = true;  // fallthrough
          }
          if _skTemp1 || _skTemp0 == 3 {
            a = bool(c);
            // fallthrough
          }
          ok = a;
        }
        case 0 {
          ;
        }
        case default {}
      }
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
