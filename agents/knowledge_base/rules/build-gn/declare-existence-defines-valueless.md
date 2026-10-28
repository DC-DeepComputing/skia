---
name: skia/build-gn/declare-existence-defines-valueless
description: When declaring existence-checked boolean preprocessor flags in GN or Bazel defines lists, declare them as valueless strings rather than appending =1
rubric: readability
severity: should_fix
---

**Rule.** In GN and Bazel build files, declare boolean preprocessor flags
checked solely for existence via `#if defined(...)` or `#ifdef` as valueless
strings (such as `"SK_DISABLE_TRACING"` or `"SKVX_DISABLE_SIMD"`) in `defines`
lists. Do not assign an explicit `=1` (such as `"MACRO=1"`) unless the macro is
checked for a specific numeric value or is a shared library export macro.

**Why.** Existence-checked macros only require being defined; the compiler
preprocessor treats `-DMACRO` as defined. Appending `=1` is redundant, deviates
from established build conventions across the codebase, and misleadingly implies
the macro is evaluated numerically rather than checked via `#if defined(...)`.

## Examples

Bad:

```cpp
# gn/toolchain/wasm.gni
wasm_defines = [
  "SKVX_DISABLE_SIMD=1",
]

# BUILD.gn
optional("png_encode") {
  public_defines = [ "SK_PNG_ENCODER_USE_CONVERT_PIXELS=1" ]
}
```

Good:

```cpp
# gn/toolchain/wasm.gni
wasm_defines = [
  "SKVX_DISABLE_SIMD",
]

# BUILD.gn
optional("png_encode") {
  public_defines = [ "SK_PNG_ENCODER_USE_CONVERT_PIXELS" ]
}
```
