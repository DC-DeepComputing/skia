### Compilation failed:

error: :8:27 error: entry point 'fragmentMain' references multiple variables that use the same resource binding @group(0), @binding(0)
@group(0) @binding(0) var tˢ: sampler;

:9:27 note: first resource binding usage declared here
@group(0) @binding(0) var tᵗ: texture_2d<f32>;


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(0) var tˢ: sampler;
@group(0) @binding(0) var tᵗ: texture_2d<f32>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    var c: vec4<f32> = textureSample(tᵗ, tˢ, vec2<f32>(0.0));
    let _skTemp0 = vec3<f32>(1.0);
    (*_stageOut).sk_FragColor = c * textureSample(tᵗ, tˢ, _skTemp0.xy / _skTemp0.z);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
