---
name: skia/cross-reference-duplicate-explanatory-comments
description: When documenting identical workarounds or invariants across related functions, state the detailed rationale once and cross-reference it rather than duplicating comments
rubric: readability
severity: nit
---

**Rule.** When documenting complex invariants, hardware or driver bug
workarounds, or subtle math across multiple related functions or call sites,
write the full explanation once in the primary function or central definition.
In secondary locations, provide a concise summary with a cross-reference
pointing to the primary explanation (e.g., 'see <function> for explanation')
rather than duplicating multi-line comment blocks.

**Why.** Duplicating lengthy explanatory comments across sibling functions
introduces visual noise and creates maintenance debt, as duplicate comments
inevitably drift out of sync when the workaround or invariant evolves.
Centralizing the full explanation at the primary site keeps comments DRY and
maintains a single canonical source of truth.

## Examples

Bad:

```cpp
// In secondary function:
// Calculate a rotated cornerID value to use for indexing within vectors that would normally
// require ".yzwx" swizzling before indexing (e.g., if `var` is a container of 4,
// `var.yzwx[cornerID]` is instead handled as `var[nextID]`). Though theoretically equivalent,
// using the modified cornerID gets around a driver bug within Qualcomm Adreno 620 GPUs
// which leads to incorrect output when swizzling and indexing into a vector within one line.
uint nextID = (cornerID + 1) % 4;
```

Good:

```cpp
// In secondary function:
// Calculate a rotated cornerID value to use for indexing within vectors that would normally
// require ".yzwx" swizzling (see similar note in analytic_rrect_vertex_fn for explanation).
uint nextID = (cornerID + 1) % 4;
```
