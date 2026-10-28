---
name: skia/rust/omit-redundant-parser-prevalidation
description: When delegating input buffers to external parser or decoder libraries, omit redundant manual pre-validation checks like buffer lengths or magic bytes
rubric: readability
severity: should_fix
---

**Rule.** When passing input buffers or byte streams to an underlying parser,
codec, or decoding library that validates input invariants internally, omit
manual pre-validation checks (such as buffer length bounds or magic header
signatures). Delegate parsing directly to the library and handle validation
failures through its returned error status or Result.

**Why.** Duplicating manual pre-validation checks before delegating to an
underlying parser introduces redundant code and creates maintenance hazards if
caller checks diverge from the parser's internal validation rules. Trusting the
parser's internal validation simplifies control flow and prevents premature
rejections.

## Examples

Bad:

```cpp
const ICC_SIGNATURE: &[u8; 4] = b"acsp";
if data.is_empty() || data.len() < 40 || &data[36..40] != ICC_SIGNATURE {
    return false;
}
match ColorProfile::new_from_slice(data) {
    Ok(profile) => profile,
    Err(_) => return false,
}
```

Good:

```cpp
// Delegate directly to parser library; it validates length and magic bytes
let profile = match ColorProfile::new_from_slice(data) {
    Ok(profile) => profile,
    Err(_) => return false,
};
```
