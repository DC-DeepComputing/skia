enable f16;

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f16>,
    colorRed: vec4<f16>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _outParamHelper_0_out_params_are_distinct_bhh(x: ptr<function, f16>, _unused1: ptr<function, f16>) -> bool {
    var _var0: f16;
    var _var1: f16;
    var _return: bool = out_params_are_distinct_bhh(&_var0, &_var1);
    (*x) = _var0;
    (*x) = _var1;
    return _return;
}
fn out_params_are_distinct_bhh(x: ptr<function, f16>, y: ptr<function, f16>) -> bool {
    (*x) = 1.0;
    (*y) = 2.0;
    return (*x) == 1.0 && (*y) == 2.0;
}
fn main(coords: vec2<f32>) -> vec4<f16> {
    var x: f16 = 0.0;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_outParamHelper_0_out_params_are_distinct_bhh(&x, &x)));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
