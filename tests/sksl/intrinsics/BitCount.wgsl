diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: i32,
  b: u32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor.x = f32(countOneBits(_globalUniforms.a)) + f32(countOneBits(_globalUniforms.b));
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(countOneBits(vec2<i32>(_globalUniforms.a))) + vec2<f32>(countOneBits(vec2<u32>(_globalUniforms.b)))), (*_stageOut).sk_FragColor.zw);
    (*_stageOut).sk_FragColor = vec4<f32>((vec3<f32>(countOneBits(vec3<i32>(_globalUniforms.a))) + vec3<f32>(countOneBits(vec3<u32>(_globalUniforms.b)))), (*_stageOut).sk_FragColor.w);
    (*_stageOut).sk_FragColor = vec4<f32>(countOneBits(vec4<i32>(_globalUniforms.a))) + vec4<f32>(countOneBits(vec4<u32>(_globalUniforms.b)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
