enable f16;

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f16>,
    colorRed: vec4<f16>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat4x4f16_diagonal(x: f16) -> mat4x4<f16> {
    return mat4x4<f16>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn main(_stageOut: ptr<function, FSOut>) {
    var ok: bool = true;
    ok = ok && !(_globalUniforms.colorGreen.y == 1.0);
    var m: mat4x4<f16> = mat4x4f16_diagonal(1.0);
    (*_stageOut).sk_FragColor = select(_globalUniforms.colorRed, (-1.0 * m) * -_globalUniforms.colorGreen, vec4<bool>(!(_globalUniforms.colorGreen.x == 1.0)));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
