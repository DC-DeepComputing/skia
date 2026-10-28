---
name: skia/ports/no-legacy-factories-for-new-backends
description: When implementing font manager factories for new backends like Fontations, do not add legacy implicit-dependency overloads; require callers to inject dependencies explicitly
rubric: design
severity: should_fix
---

**Rule.** When deprecating legacy implicit-backend factory functions (such as
font manager constructors taking default font scanners) in favor of explicit
dependency injection, do not introduce new legacy-style default overloads for
new backends like Fontations. Require callers to construct and pass dependencies
(such as std::unique_ptr<SkFontScanner>) explicitly.

**Why.** Legacy factory functions that omit dependencies exist solely for
backward compatibility while migrating existing callers of established backends.
Introducing equivalent implicit-backend overloads for newly added backends
perpetuates deprecated API patterns and contradicts the architectural migration
toward explicit dependency injection.

## Examples

Bad:

```cpp
// Bad: Adding a legacy-style single-argument overload for a new backend (Fontations)
#if !defined(SK_DISABLE_LEGACY_FONTCONFIG_FACTORY)
sk_sp<SkFontMgr> SkFontMgr_New_FontConfig(FcConfig* fc) {
    return SkFontMgr_New_FontConfig(fc, SkFontScanner_Make_Fontations());
}
#endif
```

Good:

```cpp
// Good: Require callers to explicitly construct and inject the scanner dependency
auto scanner = SkFontScanner_Make_Fontations();
auto fontMgr = SkFontMgr_New_FontConfig(fc, std::move(scanner));
```
