---
name: skia/rust/propagate-conversion-validation-failure
description: When implementing fallible data conversion or FFI helpers that populate out-parameters, return bool to propagate validation failures instead of returning void
rubric: correctness
severity: should_fix
---

**Rule.** Helper and conversion functions that validate inputs while populating
out-parameters (such as C/C++ structs across FFI boundaries) must return a
boolean (or error status) indicating success and propagate failures up the call
stack. Do not declare such fallible conversion helpers as returning void or
silently return early on validation failure, leaving destination structs
uninitialized or zeroed while callers falsely assume success.

**Why.** Returning void from fallible conversion helpers hides validation errors
from callers. If callers track presence flags (such as has_a2b) or assume
successful translation, early returns leave destination structs partially
initialized or zeroed while the overall conversion incorrectly reports success.

## Examples

Bad:

```cpp
static void ToSkcmsA2B(const rust_icc::A2B& rust_a2b, skcms_A2B* out_skcms) {
    memset(out_skcms, 0, sizeof(skcms_A2B));
    if (rust_a2b.input_channels < 1 || rust_a2b.input_channels > 4) {
        return; // Caller assumes success with a zeroed or partially populated struct
    }
    // ...
}

// Caller:
if (rust_profile.has_a2b) {
    ToSkcmsA2B(rust_profile.a2b, &out_skcms->A2B);
}
```

Good:

```cpp
static bool ToSkcmsA2B(const rust_icc::A2B& rust_a2b, skcms_A2B* out_skcms) {
    memset(out_skcms, 0, sizeof(skcms_A2B));
    if (rust_a2b.input_channels < 1 || rust_a2b.input_channels > 4) {
        return false;
    }
    // ...
    return true;
}

// Caller:
if (rust_profile.has_a2b) {
    if (!ToSkcmsA2B(rust_profile.a2b, &out_skcms->A2B)) {
        return false;
    }
}
```
