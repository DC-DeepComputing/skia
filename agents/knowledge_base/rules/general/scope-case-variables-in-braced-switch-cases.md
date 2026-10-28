---
name: skia/scope-case-variables-in-braced-switch-cases
description: When declaring variables needed only within individual switch cases, wrap the case in braces rather than hoisting declarations outside the switch
rubric: readability
severity: should_fix
---

**Rule.** Do not declare variables outside a switch statement when they are only
needed locally within individual cases. Wrap the relevant switch case in curly
braces ('case <value>: { ... } break;') to keep variable scope and lifetime
local to that case.

**Why.** Declaring case-specific variables outside the switch statement
needlessly widens their scope and lifetime across all other cases, increasing
the risk of uninitialized reads or accidental state leakage across loop
iterations. Wrapping the case in braces creates a distinct lexical block that
isolates local variable lifetimes and conforms to Skia style conventions.

## Examples

Bad:

```cpp
bool isDegenerate;
switch (verb) {
    case SkPathVerb::kLine:
        isDegenerate = SkPath::IsLineDegenerate(pts[0], pts[1], true);
        if (isDegenerate) {
            return;
        }
        break;
    case SkPathVerb::kQuad:
        ...
        break;
}
```

Good:

```cpp
switch (verb) {
    case SkPathVerb::kLine: {
        bool isDegenerate = SkPath::IsLineDegenerate(pts[0], pts[1], true);
        if (isDegenerate) {
            return;
        }
    } break;
    case SkPathVerb::kQuad:
        ...
        break;
}
```
