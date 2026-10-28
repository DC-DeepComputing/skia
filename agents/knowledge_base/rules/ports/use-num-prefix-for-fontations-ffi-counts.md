---
name: skia/ports/use-num-prefix-for-fontations-ffi-counts
description: In Fontations Rust FFI bridge functions, name count accessors with num_ and return unsigned counts directly instead of using out-parameters
rubric: readability
severity: should_fix
---

**Rule.** In Fontations Rust FFI bridge declarations and helper functions,
expose item count queries by returning an unsigned count directly (such as usize
or u16) named with the num_<entity> prefix (such as num_axes, num_glyphs,
num_named_instances, or num_color_stops). Avoid using mutable out-parameters
with boolean status returns, and avoid naming count accessors with
number_<entity> or collection-style suffixes.

**Why.** Returning an unsigned count directly simplifies caller call sites and
naturally models absent items or non-variable fonts with a count of 0 (e.g. via
unwrap_or_default()). Uniform num_<entity> naming matches font engine
conventions (such as FreeType) and ensures predictable symbol discovery across
C++ and Rust.

## Examples

Bad:

```cpp
// Avoid number_* prefix:
fn number_axes(font_ref: &BridgeFontRef) -> usize;
fn number_glyphs(font_ref: &BridgeFontRef) -> u16;

// Avoid out-parameters and collection names for count queries:
unsafe fn named_instances_collection(font_ref: &BridgeFontRef, num_instances: &mut i32) -> bool;
```

Good:

```cpp
// Use num_* prefix and return unsigned counts directly:
fn num_axes(font_ref: &BridgeFontRef) -> usize;
fn num_glyphs(font_ref: &BridgeFontRef) -> u16;
unsafe fn num_named_instances(font_ref: &BridgeFontRef) -> usize;
```
