---
name: skia/rust-prefer-let-else-for-early-returns
description: When extracting inner values from Result or Option and returning early on mismatch in Rust, prefer let-else statements over verbose match expressions
rubric: readability
severity: nit
---

**Rule.** In Rust, prefer let-else statements (e.g. let Ok(val) = expr else {
return ...; }) over multi-line match expressions when extracting an inner value
and returning early or diverging on pattern mismatch. Keep the extracted
variable in scope at the outer indentation level without nested blocks.

**Why.** Using let-else eliminates boilerplate match statements that only
extract a payload or return early, keeping the extracted variable in the outer
scope and avoiding unnecessary indentation or nested control flow.

## Examples

Bad:

```cpp
let profile = match ColorProfile::new_from_slice(data) {
    Ok(profile) => profile,
    Err(_) => return false,
};
```

Good:

```cpp
let Ok(profile) = ColorProfile::new_from_slice(data) else {
    return false;
};
```
