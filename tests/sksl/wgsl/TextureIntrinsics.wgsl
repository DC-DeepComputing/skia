diagnostic(off, derivative_uniformity);
@group(0) @binding(0) var dest: texture_storage_2d<rgba8unorm, write>;
fn _skslMain() {
  {
    let _skTemp0 = textureDimensions(dest);
    var sizeX: u32 = _skTemp0.x;
    let _skTemp1 = textureDimensions(dest);
    var sizeY: u32 = _skTemp1.y;
    {
      var y: u32 = 0u;
      loop {
        if y < sizeY {
          {
            {
              var x: u32 = 0u;
              loop {
                if x < sizeX {
                  {
                  }
                } else {
                  break;
                }
                continuing {
                  x = x + u32(1);
                }
              }
            }
          }
        } else {
          break;
        }
        continuing {
          y = y + u32(1);
        }
      }
    }
  }
}
@compute @workgroup_size(1, 1, 1) fn main() {
  _skslMain();
}
