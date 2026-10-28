---
name: skia/ports/return-zero-for-font-attribute-counts
description: When querying font attribute counts like variation axes or named instances in font backends, return unsigned zero rather than negative sentinels like -1
rubric: correctness
severity: should_fix
---

**Rule.** When querying or reporting the count of font attributes such as
variation axes or named instances in font backends or font scanner FFI bindings,
return an unsigned count (such as usize or size_t) defaulting to 0 rather than a
negative error sentinel like -1. Treat fonts lacking variation tables or
attributes as valid zero-count fonts, defaulting absent counts to 0 (e.g. using
unwrap_or_default()).

**Why.** Non-variable fonts legitimately contain zero variation axes and zero
named instances. Returning -1 on absence treats a standard, valid non-variable
font as an error condition and forces callers to handle negative sentinel
values, whereas returning an unsigned count defaulting to 0 accurately models
non-variable fonts and avoids signed/unsigned conversion issues.

## Examples

Bad:

```cpp
fn number_axes(font_ref: &BridgeFontRef) -> isize {
    font_ref
        .with_font(|f| {
            let axes = f.axes();
            isize::try_from(axes.len()).ok()
        })
        .unwrap_or(-1)
}
```

Good:

```cpp
fn num_axes(font_ref: &BridgeFontRef) -> usize {
    font_ref
        .with_font(|f| Some(f.axes().len()))
        .unwrap_or_default()
}
```
