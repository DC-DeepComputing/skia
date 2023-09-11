### Compilation failed:

error: :9:5 error: unresolved address space 'pixel_local'
var<pixel_local> pls: PixelLocalData;
    ^^^^^^^^^^^

:9:5 note: Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup'
var<pixel_local> pls: PixelLocalData;
    ^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct PixelLocalData {
  i: i32,
  f: f32,
};
var<pixel_local> pls: PixelLocalData;
fn _skslMain() {
  {
    pls.i = pls.i + i32(1);
    pls.f = pls.f * 2.0;
  }
}
@fragment fn main(_stageIn: FSIn) {
  _skslMain();
}

1 error
