---
name: skia/clear-inapplicable-out-parameters-to-empty
description: When populating an out-parameter in branches or fallback paths where the output is inapplicable, assign its empty default state ({}) rather than an arbitrary value
rubric: readability
severity: nit
---

**Rule.** When an out-parameter (such as a bounds rect, key, or configuration
descriptor) is not applicable or unused in a particular branch or fallback path,
set it to its empty or default state (`{}` or `Type::MakeEmpty()`). Do not
populate it with an arbitrary, misleading, or partial value that could lead
callers or readers to assume a meaningful result was produced.

**Why.** Assigning an arbitrary non-empty value to an out-parameter in a branch
where it is meaningless misleads callers and readers into assuming the data is
valid or relevant. Explicitly clearing it to its empty or default state
unambiguously signals that no output was produced for that code path.

## Examples

Bad:

```cpp
if (includeBounds) {
    *keyBounds = maskDeviceBounds.makeOffset(-origin.x(), -origin.y());
} else {
    // Misleading: maskDeviceBounds is unclipped and not applicable here
    *keyBounds = maskDeviceBounds;
}
```

Good:

```cpp
if (includeBounds) {
    *keyBounds = maskDeviceBounds.makeOffset(-origin.x(), -origin.y());
} else {
    // Explicitly empty: clearly signals that no bounds are produced
    *keyBounds = {};
}
```
