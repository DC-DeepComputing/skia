### Compilation failed:

error: :12:27 error: entry point 'fragmentMain' references multiple variables that use the same resource binding @group(0), @binding(0)
@group(0) @binding(0) var sˢ: sampler;

:13:27 note: first resource binding usage declared here
@group(0) @binding(0) var sᵗ: texture_2d<f32>;


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorXform: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
@group(0) @binding(0) var sˢ: sampler;
@group(0) @binding(0) var sᵗ: texture_2d<f32>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    var tmpColor: vec4<f32>;
    tmpColor = vec4<f32>(textureSample(sᵗ, sˢ, vec2<f32>(1.0)));
    let _skTemp0 = clamp((_globalUniforms.colorXform * vec4<f32>(tmpColor.xyz, 1.0)).xyz, vec3<f32>(0.0), vec3<f32>(tmpColor.w));
    let _skTemp1 = mat4x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    (*_stageOut).sk_FragColor = vec4<f32>(select(tmpColor, vec4<f32>(_skTemp0, tmpColor.w), vec4<bool>((any(_globalUniforms.colorXform[0] != _skTemp1[0]) || any(_globalUniforms.colorXform[1] != _skTemp1[1]) || any(_globalUniforms.colorXform[2] != _skTemp1[2]) || any(_globalUniforms.colorXform[3] != _skTemp1[3])))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
