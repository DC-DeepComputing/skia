enable f16;

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f16>,
};
fn main(_stageOut: ptr<function, FSOut>) {
    (*_stageOut).sk_FragColor = vec4<f16>(1.0);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
