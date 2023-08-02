### Compilation failed:

error: :8:27 error: entry point 'fragmentMain' references multiple variables that use the same resource binding @group(0), @binding(0)
@group(0) @binding(0) var test2Dˢ: sampler;

:9:27 note: first resource binding usage declared here
@group(0) @binding(0) var test2Dᵗ: texture_2d<f32>;


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(0) var test2Dˢ: sampler;
@group(0) @binding(0) var test2Dᵗ: texture_2d<f32>;
@group(0) @binding(0) var test2DRectˢ: sampler;
@group(0) @binding(0) var test2DRectᵗ: texture_2d<f32>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = textureSample(test2Dᵗ, test2Dˢ, vec2<f32>(0.5));
    (*_stageOut).sk_FragColor = textureSample(test2DRectᵗ, test2DRectˢ, vec2<f32>(0.5));
    let _skTemp0 = vec3<f32>(0.5);
    (*_stageOut).sk_FragColor = textureSample(test2DRectᵗ, test2DRectˢ, _skTemp0.xy / _skTemp0.z);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
