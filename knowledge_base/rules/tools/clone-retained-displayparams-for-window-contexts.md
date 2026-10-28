---
name: skia/tools/clone-retained-displayparams-for-window-contexts
description: When passing persistent member configuration like DisplayParams to window context factories or consuming child objects, pass ->clone() instead of std::move() to avoid nullifying parent state
rubric: correctness
severity: should_fix
---

**Rule.** When passing persistent member configuration (such as
fRequestedDisplayParams) to window context factory functions (e.g.,
skwindow::Make*For*) or child objects that consume unique ownership, pass a copy
via ->clone() rather than moving the member with std::move(). Retain the
persistent configuration pointer intact across the window's lifecycle so
subsequent queries and context recreation succeed.

**Why.** Moving from fRequestedDisplayParams empties the parent Window's member
pointer. Subsequent configuration queries or native context recreation events
(such as Android surface recreation) will dereference a null pointer and crash.

## Examples

Bad:

```cpp
// Moving nullifies fRequestedDisplayParams, breaking subsequent recreation.
fWindowContext = skwindow::MakeGLForAndroid(window, std::move(fRequestedDisplayParams));
```

Good:

```cpp
// Clone preserves fRequestedDisplayParams for future queries and context recreation.
fWindowContext = skwindow::MakeGLForAndroid(window, fRequestedDisplayParams->clone());
```
