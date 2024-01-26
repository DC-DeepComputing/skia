diagnostic(off, derivative_uniformity);
struct IndirectDispatchArgs {
  x: i32,
  y: i32,
  z: i32,
};
fn _skslMain() {
  {
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
