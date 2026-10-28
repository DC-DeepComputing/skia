---
name: skia/prefer-else-if-over-nested-if-in-else
description: When an else block contains only a nested if or an early-exit guard before trailing logic, condense it into an else if branch
rubric: readability
severity: nit
---

**Rule.** When an else block contains only a single nested if statement,
condense it into an else if branch rather than nesting the condition inside the
else body. Similarly, when an else block begins with an early-exit guard (such
as if (cond) continue;) before trailing statements in a loop body, invert the
guard into an else if (!cond) branch to flatten control flow and eliminate
unnecessary nesting. Preserve mutually exclusive control flow at a flat
indentation level, reserving trailing else for the final fallback path.

**Why.** Nesting an if statement or an early-exit guard inside an else block
adds unnecessary indentation depth and obscures that the branches represent
mutually exclusive alternatives. Inverting or condensing the nested condition
into an else if branch flattens control flow, eliminates branching indirection,
and directly conveys when trailing logic executes.

## Examples

Bad:

```cpp
// Nested single if inside else:
if (table_entry.to_xyzd50) {
    table_entry_to_xyzd50 = *table_entry.to_xyzd50;
} else {
    if (!table_entry.sk_primaries.toXYZD50(&table_entry_to_xyzd50)) {
        continue;
    }
}

// Nested early-exit guard before trailing statements in loop body:
if (usePairs && param->isSampler()) {
    emitSamplerType(param);
} else {
    if (paramIsSpecialized) {
        continue;
    }
    emitDefaultType(param);
}
```

Good:

```cpp
// Condensed into else if:
if (table_entry.to_xyzd50) {
    table_entry_to_xyzd50 = *table_entry.to_xyzd50;
} else if (!table_entry.sk_primaries.toXYZD50(&table_entry_to_xyzd50)) {
    continue;
}

// Inverted early-exit guard into else if:
if (usePairs && param->isSampler()) {
    emitSamplerType(param);
} else if (!paramIsSpecialized) {
    emitDefaultType(param);
}
```
