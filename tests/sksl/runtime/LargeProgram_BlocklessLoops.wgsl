diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    var i: i32;
    {
      var a: i32 = 0;
      while (a < 10) {
        {
          var b: i32 = 0;
          while (b < 10) {
            {
              var c: i32 = 0;
              while (c < 10) {
                {
                  var d: i32 = 0;
                  while (d < 10) {
                    i = i + i32(1);
                    d = d + i32(1);
                  }
                }
                c = c + i32(1);
              }
            }
            b = b + i32(1);
          }
        }
        a = a + i32(1);
      }
    }
    return vec4<f16>(0.0h);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
