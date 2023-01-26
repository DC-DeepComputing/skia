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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main() -> vec4<f16> {
    return vec4<f16>(f16(_globalUniforms._array[0].x), f16(_globalUniforms.vector.y), f16(_globalUniforms.matrix[0].y), 1.0);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main();
    return _stageOut;
}
