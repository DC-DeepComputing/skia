---
name: skia/omit-redundant-break-after-return
description: When returning directly from a switch case, omit redundant trailing break statements that form unreachable dead code
rubric: readability
severity: nit
---

**Rule.** Do not place a break; statement immediately following a return
statement in a switch case. Rely on the return statement to terminate control
flow and exit the function directly, omitting unreachable dead code.

**Why.** In C++, a return statement unconditionally exits the enclosing
function, making any subsequent break; within that switch case unreachable dead
code. Omitting superfluous break statements eliminates visual clutter and
conforms to project conventions.

## Examples

Bad:

```cpp
switch (format) {
    case Format::kRGB:
        return 3;
        break;
    case Format::kRGBA: return 4; break;
    default:
        return 0;
        break;
}
```

Good:

```cpp
switch (format) {
    case Format::kRGB:
        return 3;
    case Format::kRGBA: return 4;
    default:
        return 0;
}
```
