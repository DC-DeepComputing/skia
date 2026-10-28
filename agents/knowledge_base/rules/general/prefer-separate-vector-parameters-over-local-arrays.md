---
name: skia/prefer-separate-vector-parameters-over-local-arrays
description: When passing subsets of matrix columns or array elements to SkSL helpers, pass separate vector parameters instead of fixed-size arrays
rubric: safety
severity: should_fix
---

**Rule.** In SkSL helper functions, prefer taking separate vector parameters
(such as half4 c0, half4 c1) rather than fixed-size arrays (such as half4 c[2])
when callers pass subsets of matrix columns or array elements. Avoid declaring
local temporary arrays in caller code solely to stage and copy array elements
before invoking a helper.

**Why.** SkSL does not support array slicing, sub-range views, or pointer spans.
Requiring an array parameter forces callers to declare temporary local arrays
and copy elements into them one by one, adding boilerplate and risking extra
copy overhead or register pressure on GPU shader compilers with poor copy
elision.

## Examples

Bad:

```cpp
$pure float3 $apply_xfer_fn(int kind, float3 x, half4 cs[2]) { ... }

// Call site: temporary array required to pass subset of matrix/array
half4 srcCoeffs[2];
srcCoeffs[0] = coeffs[0];
srcCoeffs[1] = coeffs[1];
color.rgb = $apply_xfer_fn(srcKind, color.rgb, srcCoeffs);
```

Good:

```cpp
$pure float3 $apply_xfer_fn(int kind, float3 x, half4 coeffs0, half4 coeffs1) { ... }

// Call site: pass vector elements directly without local temporary array
color.rgb = $apply_xfer_fn(srcKind, color.rgb, coeffs[0], coeffs[1]);
```
