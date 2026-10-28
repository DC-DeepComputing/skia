---
name: skia/gm/define-gms-at-file-scope-without-skiagm-namespace
description: When authoring GM tests or registering them with DEF_GM or DEF_SIMPLE_GM, define tests at file scope instead of wrapping implementations in namespace skiagm
rubric: readability
severity: nit
---

**Rule.** Do not wrap GM test files, classes, or macro registrations (such as
DEF_GM and DEF_SIMPLE_GM) in namespace skiagm { ... }. Define GM tests and
file-local helpers at global file scope (using static or anonymous namespaces
for internal linkage), and explicitly qualify GM framework types such as
skiagm::GM or skiagm::DrawResult only where needed.

**Why.** GM registration macros expand at file scope and already qualify
framework internals. Wrapping tests in namespace skiagm pollutes the shared
framework namespace with translation-unit helpers and constants, and conflicts
with modern Skia GM conventions that maintain test implementations at global
scope.

## Examples

Bad:

```cpp
namespace skiagm {

static constexpr size_t kScale = 4;
static void draw_scaled_path(SkCanvas* canvas, const SkPath& path, const SkPaint& paint) {
    ...
}

DEF_SIMPLE_GM(hairlines_buttcap, canvas, 100, 500) {
    ...
}

}  // namespace skiagm
```

Good:

```cpp
static constexpr size_t kScale = 4;
static void draw_scaled_path(SkCanvas* canvas, const SkPath& path, const SkPaint& paint) {
    ...
}

DEF_SIMPLE_GM(hairlines_buttcap, canvas, 100, 500) {
    ...
}
```
