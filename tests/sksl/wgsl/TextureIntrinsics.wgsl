diagnostic(off, derivative_uniformity);
@group(0) @binding(0) var dest: texture_storage_2d<rgba8unorm, write>;
fn _skslMain() {
  {
  }
}
@compute @workgroup_size(1, 1, 1) fn main() {
  _skslMain();
}
