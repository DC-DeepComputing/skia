### Compilation failed:

error: :6:5 error: 'pixel_local' address space requires the 'chromium_experimental_pixel_local' extension enabled
var<pixel_local> pls: PixelLocalData;
    ^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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
@fragment fn main() {
  _skslMain();
}

1 error
