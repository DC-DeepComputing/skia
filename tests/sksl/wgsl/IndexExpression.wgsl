enable f16;

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
    _array: array<vec4<f32>, 5>,
    vector: vec2<f32>,
    matrix: mat2x2<f32>,
    idx: i32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main() -> vec4<f16> {
    return vec4<f16>(f16(_globalUniforms._array[_globalUniforms.idx][_globalUniforms.idx]), f16(_globalUniforms.vector[_globalUniforms.idx + 1]), f16(_globalUniforms.matrix[_globalUniforms.idx][_globalUniforms.idx + 1]), 1.0);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main();
    return _stageOut;
}
