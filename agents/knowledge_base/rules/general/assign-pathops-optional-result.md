---
name: skia/assign-pathops-optional-result
description: When modifying or accumulating paths using SkPathOps functions, assign the returned optional path and preserve accumulated state rather than falling back to empty
rubric: correctness
severity: should_fix
---

**Rule.** When performing path operations or simplifications using SkPathOps
functions (such as Op, Simplify, or AsWinding), assign the returned
std::optional<SkPath> back to the destination path rather than expecting
in-place mutation. When accumulating paths (such as in a loop) or modifying a
path where previous progress must be retained, conditionally update on success
(e.g., if (auto result = Op(...)) { path = *result; }) or fall back to the
existing path (.value_or(path)) rather than falling back to an empty SkPath via
.value_or(SkPath()), which discards accumulated state on failure.

**Why.** SkPathOps functions return a fallible std::optional<SkPath> rather than
mutating arguments in place. Failing to assign the return value leaves the
destination path unmodified, while falling back to an empty or default SkPath
upon failure wipes out all previously accumulated path data.

## Examples

Bad:

```cpp
// Bad: Op does not mutate in place; omitting assignment leaves 'path' unmodified:
Op(path, fillPath, kUnion_SkPathOp);

// Bad: Falling back to SkPath() in a loop discards previously accumulated progress:
SkPath accumulated;
for (const auto& child : children) {
    accumulated = Op(accumulated, child.asPath(), kUnion_SkPathOp).value_or(SkPath());
}
```

Good:

```cpp
// Good: Assign returned optional back to path with conditional check or fallback:
if (auto result = Op(path, fillPath, op)) {
    path = *result;
}

// Good: In accumulation loops, preserve prior progress on failure:
SkPath accumulated;
for (const auto& child : children) {
    if (auto result = Op(accumulated, child.asPath(), kUnion_SkPathOp)) {
        accumulated = *result;
    }
}
// Or: accumulated = Op(accumulated, child.asPath(), kUnion_SkPathOp).value_or(accumulated);
```
