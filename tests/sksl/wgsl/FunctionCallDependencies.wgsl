enable f16;

struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f16>,
};
fn getColor_h4h(c: f16) -> vec4<f16> {
    return vec4<f16>(c);
}
fn getFragCoordAugmentedColor_h4h(_stageIn: FSIn, c: f16) -> vec4<f16> {
    return vec4<f16>(_stageIn.sk_FragCoord.xyxy * vec4<f32>(getColor_h4h(c)));
}
fn writeColorToOutput_vh(_stageOut: ptr<function, FSOut>, c: f16) {
    (*_stageOut).sk_FragColor = getColor_h4h(c);
}
fn writeToOutput_v(_stageOut: ptr<function, FSOut>) {
    writeColorToOutput_vh(_stageOut, 1.0);
}
fn modifyOutputColor_v(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
    (*_stageOut).sk_FragColor += getFragCoordAugmentedColor_h4h(_stageIn, 2.0);
}
fn main(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
    writeToOutput_v(_stageOut);
    modifyOutputColor_v(_stageIn, _stageOut);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(_stageIn, &_stageOut);
    return _stageOut;
}
