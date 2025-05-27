diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const x1: f32 = 1.0;
    const x2: vec2<f32> = vec2<f32>(2.0);
    const x3: vec3<f32> = vec3<f32>(3.0);
    const x4: vec4<f32> = vec4<f32>(4.0);
    const x5: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const x6: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const x7: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    const x8: f32 = 1.0;
    const x9: vec2<f32> = vec2<f32>(2.0);
    const x10: vec3<f32> = vec3<f32>(3.0);
    const x11: vec4<f32> = vec4<f32>(4.0);
    const x12: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const x13: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const x14: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    const x15: bool = true;
    const x16: vec2<bool> = vec2<bool>(true);
    const x17: vec3<bool> = vec3<bool>(true);
    const x18: vec4<bool> = vec4<bool>(true);
    const x19: i32 = 1;
    const x20: vec2<i32> = vec2<i32>(2);
    const x21: vec3<i32> = vec3<i32>(3);
    const x22: vec4<i32> = vec4<i32>(4);
    const _skTemp0 = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const _skTemp1 = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const _skTemp2 = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    const _skTemp3 = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const _skTemp4 = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const _skTemp5 = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((((x1 == 1.0) && all(x2 == vec2<f32>(2.0))) && all(x3 == vec3<f32>(3.0))) && all(x4 == vec4<f32>(4.0))) && (all(x5[0] == _skTemp0[0]) && all(x5[1] == _skTemp0[1]))) && (all(x6[0] == _skTemp1[0]) && all(x6[1] == _skTemp1[1]) && all(x6[2] == _skTemp1[2]))) && (all(x7[0] == _skTemp2[0]) && all(x7[1] == _skTemp2[1]) && all(x7[2] == _skTemp2[2]) && all(x7[3] == _skTemp2[3]))) && (x8 == 1.0)) && all(x9 == vec2<f32>(2.0))) && all(x10 == vec3<f32>(3.0))) && all(x11 == vec4<f32>(4.0))) && (all(x12[0] == _skTemp3[0]) && all(x12[1] == _skTemp3[1]))) && (all(x13[0] == _skTemp4[0]) && all(x13[1] == _skTemp4[1]) && all(x13[2] == _skTemp4[2]))) && (all(x14[0] == _skTemp5[0]) && all(x14[1] == _skTemp5[1]) && all(x14[2] == _skTemp5[2]) && all(x14[3] == _skTemp5[3]))) && (x15 == true)) && all(x16 == vec2<bool>(true))) && all(x17 == vec3<bool>(true))) && all(x18 == vec4<bool>(true))) && (x19 == 1)) && all(x20 == vec2<i32>(2))) && all(x21 == vec3<i32>(3))) && all(x22 == vec4<i32>(4))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
