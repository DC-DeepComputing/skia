### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testStorageBuffer {
  testArr: array<f32>,
};
@group(0) @binding(0) var<storage, read> _storage0 : testStorageBuffer;
struct testStorageBufferStruct {
  testArrStruct: array<S>,
};
@group(0) @binding(1) var<storage, read> _storage1 : testStorageBufferStruct;
struct S {
  y: i32,
};
fn unsizedInParameterD_iS(x: array<S>) -> i32 {
  {
    return x[0].y;
  }
}
fn getColor_h4f(arr: array<f32>) -> vec4<f32> {
  {
    return vec4<f32>(f32(arr[0]), f32(arr[1]), f32(arr[2]), f32(arr[3]));
  }
}
fn getColorSecond_h4f(arr: array<f32>) -> vec4<f32> {
  {
    return vec4<f32>(f32(arr[0]), f32(arr[1]), f32(arr[2]), f32(arr[3]));
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = getColor_h4f(_storage0.testArr);
    (*_stageOut).sk_FragColor = _skTemp2;
    let _skTemp3 = getColorSecond_h4f(_storage0.testArr);
    (*_stageOut).sk_FragColor = _skTemp3;
    let _skTemp4 = unsizedInParameterD_iS(_storage1.testArrStruct);
    (*_stageOut).sk_FragColor = vec4<f32>(f32(_skTemp4));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
