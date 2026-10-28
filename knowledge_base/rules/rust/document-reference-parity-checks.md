---
name: skia/rust/document-reference-parity-checks
description: When writing Rust parsers or FFI bridges that mirror reference C++ implementations, document intentional validation checks and bailouts with comments citing reference behavior
rubric: readability
severity: should_fix
---

**Rule.** In Rust codecs, format parsers, and FFI bridges written to mirror
reference C++ implementations (such as skcms or native image decoders), document
intentional validation and rejection checks with inline comments citing the
reference behavior or specification. When bailing out on unsupported variants or
strict invariants, explain whether the check enforces specification constraints
or mirrors reference implementation limits.

**Why.** Without explanatory comments citing reference behavior and
specifications, future maintainers cannot distinguish intentional strict
bailouts from missing functionality or overly restrictive checks. Clear
documentation prevents contributors from inadvertently introducing divergent
fallback paths or breaking behavioral parity with the reference C++ parser.

## Examples

Bad:

```cpp
out.connection_space = match profile.pcs {
    moxcms::DataColorSpace::Xyz => ffi::skcms_Signature::skcms_Signature_XYZ,
    moxcms::DataColorSpace::Lab => ffi::skcms_Signature::skcms_Signature_Lab,
    _ => return false,
};
```

Good:

```cpp
// Profile Connection Space (PCS) must be XYZ or Lab per ICC spec.
// skcms only supports these two PCS values. Reject profiles with other values
// to match skcms_ParseWithA2BPriority behavior.
out.connection_space = match profile.pcs {
    moxcms::DataColorSpace::Xyz => ffi::skcms_Signature::skcms_Signature_XYZ,
    moxcms::DataColorSpace::Lab => ffi::skcms_Signature::skcms_Signature_Lab,
    _ => return false, // Reject unsupported PCS to mirror skcms
};
```
