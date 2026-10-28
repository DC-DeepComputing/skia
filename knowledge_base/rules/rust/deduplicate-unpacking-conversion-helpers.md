---
name: skia/rust/deduplicate-unpacking-conversion-helpers
description: When converting data or parameter slices across multiple Rust functions or match arms, extract repeated validation and field-unpacking into a shared helper
rubric: design
severity: should_fix
---

**Rule.** When converting external crate data or parameter slices across
multiple functions or match arms in Rust FFI, extract repeated validation,
bounds checking, and field-unpacking logic into a shared helper function
returning an Option or Result. Call the helper and compose it (e.g., using
.map()) rather than duplicating slice length checks, element indexing, and
struct construction across callers.

**Why.** Duplicating parameter validation and field extraction across multiple
conversion routines inflates code size, increases boilerplate, and risks subtle
behavioral divergence if parsing requirements or bounds checks are updated in
one place but missed in another.

## Examples

Bad:

```cpp
// Duplicating bounds checks and struct initialization across multiple functions
fn convert_to_curve(curve: &ToneReprCurve) -> Option<ffi::Curve> {
    match curve {
        ToneReprCurve::Parametric(params) => {
            if params.len() >= 7 {
                Some(ffi::Curve {
                    table_entries: 0,
                    parametric: ffi::TransferFunction {
                        g: params[0], a: params[1], b: params[2],
                        c: params[3], d: params[4], e: params[5], f: params[6],
                    },
                    table_data: Vec::new(),
                })
            } else {
                None
            }
        }
        _ => None,
    }
}

fn convert_trc_to_transfer_function(curve: &ToneReprCurve) -> Option<ffi::TransferFunction> {
    match curve {
        ToneReprCurve::Parametric(params) => {
            if params.len() >= 7 {
                Some(ffi::TransferFunction {
                    g: params[0], a: params[1], b: params[2],
                    c: params[3], d: params[4], e: params[5], f: params[6],
                })
            } else {
                None
            }
        }
        _ => None,
    }
}
```

Good:

```cpp
fn params_to_transfer_function(params: &[f32]) -> Option<ffi::TransferFunction> {
    if params.len() >= 7 {
        Some(ffi::TransferFunction {
            g: params[0], a: params[1], b: params[2],
            c: params[3], d: params[4], e: params[5], f: params[6],
        })
    } else {
        None
    }
}

fn convert_to_curve(curve: &ToneReprCurve) -> Option<ffi::Curve> {
    match curve {
        ToneReprCurve::Parametric(params) => {
            params_to_transfer_function(params).map(|tf| ffi::Curve {
                table_entries: 0,
                parametric: tf,
                table_data: Vec::new(),
            })
        }
        _ => None,
    }
}

fn convert_trc_to_transfer_function(curve: &ToneReprCurve) -> Option<ffi::TransferFunction> {
    match curve {
        ToneReprCurve::Parametric(params) => params_to_transfer_function(params),
        _ => None,
    }
}
```
