diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var ok = true;
    var a = 1;
    a = a + a;
    a = a + a;
    a = a + a;
    a = a + a;
    a = a + a;
    ok = ok && (a == 32);
    var b = 10;
    b = b - 2;
    b = b - 2;
    b = b - 1;
    b = b - 3;
    ok = ok && (b == 2);
    var c = 2;
    c = c * c;
    c = c * c;
    c = c * 4;
    c = c * 2;
    ok = ok && (c == 128);
    var d = 256;
    d = d / 2;
    d = d / 2;
    d = d / 4;
    d = d / 4;
    ok = ok && (d == 4);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
