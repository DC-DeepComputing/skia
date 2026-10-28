---
name: skia/modules/verify-resolved-variable-before-js-fallbacks
description: When resolving overloaded JavaScript parameters before applying fallback defaults, check whether the resolved target variable is falsy rather than inspecting raw arguments
rubric: correctness
severity: should_fix
---

**Rule.** When parsing polymorphic or overloaded function arguments in
JavaScript shims, verify whether the resolved destination variable has been
populated (such as `if (!toDraw)`) before initializing fallback defaults. Do not
test raw input arguments (such as `if (!path)`), which can evaluate truthy for
unrecognized argument types while leaving the destination variable unassigned.

**Why.** In JavaScript functions that accept varying argument combinations, an
argument may be passed that satisfies a generic truthy check without matching
any expected type branch. Testing the raw argument instead of the resolved
target variable skips fallback initialization and leaves the variable undefined,
triggering runtime TypeErrors when accessed.

## Examples

Bad:

```cpp
var toDraw;
if (typeof path === 'string') {
  fillRule = path;
  toDraw = this._currentPath.snapshot();
} else if (path && path._getPath) {
  toDraw = path._getPath();
}
if (!path) {
  toDraw = this._currentPath.snapshot();
}
```

Good:

```cpp
var toDraw;
if (typeof path === 'string') {
  fillRule = path;
  toDraw = this._currentPath.snapshot();
} else if (path && path._getPath) {
  toDraw = path._getPath();
}
if (!toDraw) {
  toDraw = this._currentPath.snapshot();
}
```
