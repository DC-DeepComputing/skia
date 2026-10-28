---
name: skia/omit-semicolon-after-function-definition
description: When terminating statements or defining functions in C++ source and header files, omit redundant or duplicate semicolons to prevent superfluous empty statements
rubric: readability
severity: nit
---

**Rule.** Do not place redundant or duplicate semicolons at the end of
statements, nor place an extraneous semicolon after the closing brace of a
function or method definition. Reserve trailing semicolons after closing braces
for type declarations (such as structs, classes, and enums) and lambda
expressions.

**Why.** Duplicate or extraneous semicolons create superfluous empty statements
and redundant declarations at namespace or class scope. Omitting redundant
semicolons eliminates visual clutter and avoids compiler warnings such as
-Wextra-semi and -Wextra-semi-stmt.

## Examples

Bad:

```cpp
// Duplicate semicolon at end of statement:
components[3] = (CGFloat)SkColorGetA(bgra) * (1/255.0f);;

// Extraneous semicolon after function body:
static SkPath make_box(SkPoint pt) {
    SkPathBuilder pb;
    pb.addRect({pt.fX - 2, pt.fY - 2, pt.fX + 2, pt.fY + 2});
    return pb.detach();
};
```

Good:

```cpp
// Single semicolon terminating statement:
components[3] = (CGFloat)SkColorGetA(bgra) * (1/255.0f);

// Function body closed without trailing semicolon:
static SkPath make_box(SkPoint pt) {
    SkPathBuilder pb;
    pb.addRect({pt.fX - 2, pt.fY - 2, pt.fX + 2, pt.fY + 2});
    return pb.detach();
}
```
