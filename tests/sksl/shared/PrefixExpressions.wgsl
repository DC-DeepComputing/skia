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
    mask: i32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat4x4f16_diagonal(x: f16) -> mat4x4<f16> {
    return mat4x4<f16>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn main(c: vec2<f32>) -> vec4<f16> {
    var ok: bool = true;
    ok = ok && !(_globalUniforms.colorGreen.y == 1.0);
    var mask: vec2<u32> = vec2<u32>(u32(_globalUniforms.mask), ~u32(_globalUniforms.mask));
    var imask: vec2<i32> = vec2<i32>(~mask);
    mask = ~mask & vec2<u32>(~imask);
    var m: mat4x4<f16> = mat4x4f16_diagonal(f16(mask.x));
    return select(_globalUniforms.colorRed, (-1.0 * m) * -_globalUniforms.colorGreen, vec4<bool>(!(_globalUniforms.colorGreen.x == 1.0)));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
