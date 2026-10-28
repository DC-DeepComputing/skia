---
name: skia/core/forward-all-variadic-arguments-to-recursive-helper
description: When delegating variadic arguments to a recursive helper, forward all arguments directly instead of manually unpacking or inserting leading elements at the call site
rubric: design
severity: should_fix
---

**Rule.** When delegating variadic arguments to a recursive helper function that
unpacks parameter packs, forward all arguments directly to the helper. Do not
manually extract, unpack, or insert leading elements at the entry call site
before invoking the helper.

**Why.** Manually processing leading arguments before delegating to a recursive
helper duplicates insertion and conversion logic between the caller and the
helper. Passing all arguments directly into the helper centralizes pack
processing, guarantees consistent container insertion semantics, and avoids
redundant call-site boilerplate.

## Examples

Bad:

```cpp
template <typename... Args>
DebugStageBuilder& add(const SkBitmap& panel, SkRasterPipelineOp op, Args... args) {
    std::vector<SkBitmap> panels;
    std::vector<SkRasterPipelineOp> ops;
    panels.push_back(panel);
    ops.push_back(op);

    add_next(panels, ops, args...);
    fDebugStages.push_back({panels, ops});
    return *this;
}
```

Good:

```cpp
template <typename... Args>
DebugStageBuilder& add(const SkBitmap& panel, SkRasterPipelineOp op, Args... args) {
    std::vector<SkBitmap> panels;
    std::vector<SkRasterPipelineOp> ops;

    add_next(panels, ops, panel, op, args...);
    fDebugStages.push_back({panels, ops});
    return *this;
}
```
