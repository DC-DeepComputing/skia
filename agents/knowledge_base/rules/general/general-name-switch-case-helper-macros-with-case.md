---
name: skia/general-name-switch-case-helper-macros-with-case
description: When defining helper macros that expand to switch-case labels, use a _CASE or _CASES suffix rather than CHECK_ and spell out abbreviations
rubric: readability
severity: nit
---

**Rule.** Name helper macros that generate or expand switch-case branches using
a ..._CASE or ..._CASES suffix (or CASE(...)) rather than CHECK_... or
assertion-like prefixes. Additionally, avoid ambiguous abbreviations (such as CT
for ColorType) that can be confused with adjacent concepts (such as context or
ctx); spell out the domain concept explicitly.

**Why.** In Google and Skia codebases, CHECK signifies assertions and invariant
verification; naming a macro that expands to switch-case labels with CHECK_ is
misleading. In addition, ambiguous abbreviations like CT (ColorType) create
confusion when juxtaposed with context (ctx) or related concepts.

## Examples

Bad:

```cpp
#define CHECK_CT(stage_ct, sk_ct)    \
    case Op::load_##stage_ct:         \
    case Op::load_##stage_ct##_dst:   \
        ct = sk_ct;                   \
        break;
```

Good:

```cpp
#define COLOR_TYPE_CASE(stage_ct, sk_ct) \
    case Op::load_##stage_ct:             \
    case Op::load_##stage_ct##_dst:       \
        ct = sk_ct;                       \
        break;
```
