diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const kConstant = 0;
const kOtherConstant = 1;
const kAnotherConstant = 2;
const kFloatConstant = 2.14;
const kFloatConstantAlias = kFloatConstant;
const kConstVec = vec4<f16>(1.0h, 0.2h, 2.14h, 1.0h);
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    const kLocalFloatConstant = 3.14;
    let kLocalFloatConstantAlias = kLocalFloatConstant;
    let integerInput = i32(_globalUniforms.colorGreen.y);
    if integerInput == kConstant {
      {
        return vec4<f16>(2.14h);
      }
    } else {
      if integerInput == kOtherConstant {
        {
          return _globalUniforms.colorGreen;
        }
      } else {
        if integerInput == kAnotherConstant {
          {
            return kConstVec;
          }
        } else {
          if kLocalFloatConstantAlias < (f32(_globalUniforms.colorGreen.x) * kLocalFloatConstant) {
            {
              return vec4<f16>(3.14h);
            }
          } else {
            if kFloatConstantAlias >= (f32(_globalUniforms.colorGreen.x) * kFloatConstantAlias) {
              {
                return vec4<f16>(0.0h);
              }
            } else {
              {
                return vec4<f16>(1.0h, 0.0h, 0.0h, 1.0h);
              }
            }
          }
        }
      }
    }
  }
  return vec4<f16>();
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
