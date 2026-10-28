---
name: skia/remove-commented-out-code
description: 'When modifying code, remove commented-out blocks, declarations, #includes, temporary debug overrides, dead if (0) branches, and #if 0; place intentionally retained debugging code in if ((false))'
rubric: readability
severity: should_fix
---

**Rule.** Do not leave commented-out code, declarations (`// friend class`),
access specifiers (`// private:`), `#include` directives, empty conditionals,
obsolete build configurations, temporary debug overrides or short-circuit
returns (`return;`, `false || ...`, or `1 &&` / `0 ||` tautologies), dead
branches (`if (0)`), benchmarking hacks, testing holdovers, or `#if 0` blocks in
source, header, test, or build files. Restore intended access specifiers and
delete dead or disabled lines directly rather than commenting them out; if
diagnostic or debugging code is intentionally retained, enclose it in an `if
((false)) { ... }` block to prevent bitrot without compiler warnings. Restore
capability and runtime conditions bypassed during local testing, and omit
deferred APIs entirely until enabled.

**Why.** Commented-out code, declarations, access specifiers, includes,
temporary debug overrides, premature short-circuit returns, dead if (0) blocks,
benchmarking hacks, and #if 0 blocks create visual clutter, bypass pipeline
operations, obscure active dependencies, and leave behind dead or stale code.
Disabling logic by commenting out call sites or bypassing checks with hardcoded
booleans or premature returns leaves behind orphaned logic and dead members;
deleting obsolete code keeps the codebase maintainable. When debugging code must
be retained, enclosing it in if ((false)) forces the compiler to parse and
typecheck statements against evolving APIs, while the extra parentheses suppress
constant-condition compiler warnings.

## Examples

Bad:

```cpp
if (0) {
    // enable for debugging/OBB visualization
    SkPaint p;
    ctx.canvas()->drawRect(this->objectBoundingBox(ctx), p);
}

//friend class SkImage;

// Premature return and benchmarking hack leaving subsequent code dead:
*r = __lasx_xvand_v(rg, mask_00ff);
*g = __lasx_xvsrli_h(rg, 8);
return;
//nerver use
auto cast_U16 = [](U32 v) -> U16 { ... };
```

Good:

```cpp
if ((false)) {
    SkPaint p;
    ctx.canvas()->drawRect(this->objectBoundingBox(ctx), p);
}

// Obsolete declarations, dead if (0) branches, temporary returns, and dead lambdas removed:
split(rgba, &_01, &_23);
rgba = join<U32>(_02, _13);
auto cast_U16 = [](U32 v) -> U16 { ... };
```
