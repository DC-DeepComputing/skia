diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testStorageBuffer {
  testArr: array<f32>,
};
@group(0) @binding(0) var<storage, read> _storage0 : testStorageBuffer;
fn getColor_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f32> {
  {
    return vec4<f32>(f32((*arr)[0]), f32((*arr)[1]), f32((*arr)[2]), f32((*arr)[3]));
  }
}
fn helper_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f32> {
  {
    let _skTemp1 = getColor_h4f(arr);
    return _skTemp1;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = helper_h4f(&(_storage0.testArr));
    (*_stageOut).sk_FragColor = _skTemp2;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
