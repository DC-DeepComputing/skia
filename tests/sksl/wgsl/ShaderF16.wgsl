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
fn _outParamHelper_0_helper_vh(scalar: ptr<function, f16>) {
    var _var0: f16;
    helper_vh(&_var0);
    (*scalar) = _var0;
}
fn mat4x4f16_diagonal(x: f16) -> mat4x4<f16> {
    return mat4x4<f16>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn helper_vh(value: ptr<function, f16>) {
    (*value) = _globalUniforms.colorRed.x;
}
fn main(_stageOut: ptr<function, FSOut>) {
    var scalar: f16 = _globalUniforms.colorGreen.x;
    _outParamHelper_0_helper_vh(&scalar);
    var vec: vec2<f16> = _globalUniforms.colorGreen.xy;
    var mat: mat4x4<f16> = mat4x4f16_diagonal(_globalUniforms.colorGreen.y);
    (*_stageOut).sk_FragColor = mat * vec4<f16>(vec, 0.0, scalar);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
