---
name: skia/rust/reject-unsupported-icc-profile-connection-space
description: When parsing ICC profiles or wrapping profile parsers, reject unsupported Profile Connection Space signatures (non-XYZ/non-Lab) immediately instead of falling back to Unknown
rubric: correctness
severity: should_fix
---

**Rule.** When parsing ICC profiles or wrapping profile parsers, validate that
the Profile Connection Space (PCS) signature is XYZ or Lab. Immediately fail
parsing (returning `false` or `None`) if the profile specifies an unsupported or
unrecognized PCS signature rather than falling back to an `Unknown` or default
connection space.

**Why.** Per the ICC specification and skcms reference behavior
(skcms_ParseWithA2BPriority), the Profile Connection Space (PCS) must be XYZ or
Lab. Proceeding with an unknown or fallback connection space allows invalid
profiles to succeed parsing, producing invalid color transformations downstream.

## Examples

Bad:

```cpp
let connection_space = match profile.pcs {
    DataColorSpace::Xyz => ColorSpace::XYZ,
    DataColorSpace::Lab => ColorSpace::Lab,
    _ => ColorSpace::Unknown, // Fallback allows invalid profiles to pass parsing
};
```

Good:

```cpp
out.connection_space = match profile.pcs {
    moxcms::DataColorSpace::Xyz => ffi::skcms_Signature::skcms_Signature_XYZ,
    moxcms::DataColorSpace::Lab => ffi::skcms_Signature::skcms_Signature_Lab,
    _ => return false, // Reject unsupported PCS immediately
};
```
