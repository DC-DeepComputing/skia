---
name: skia/api/avoid-forwarding-factories-on-product-classes
description: When deprecating default singletons on product classes, do not add forwarding static factories accepting a manager; require callers to invoke manager methods directly
rubric: design
severity: should_fix
---

**Rule.** When deprecating or removing static factory methods on product classes
that relied on global default singletons (such as SkFontMgr::RefDefault()), do
not add redundant forwarding static factory methods that accept a manager or
factory object (e.g., SkTypeface::MakeFromFile(..., sk_sp<SkFontMgr>)) to
delegate directly to it. Direct callers that hold or instantiate the manager to
invoke its creation methods directly (e.g., mgr->makeFromFile(...)).

**Why.** Redundant forwarding factory methods create unnecessary trampoline
wrappers that bloat the public product class interface without adding
abstraction or functionality. Requiring callers to invoke the manager's methods
directly clarifies object ownership, prevents interface bloat, and aligns with
explicit dependency injection.

## Examples

Bad:

```cpp
// Redundant forwarding trampoline on product class:
class SK_API SkTypeface : public SkRefCnt {
public:
    static sk_sp<SkTypeface> MakeFromFile(const char path[], int index, sk_sp<SkFontMgr> mgr) {
        return mgr ? mgr->makeFromFile(path, index) : nullptr;
    }
};
```

Good:

```cpp
// Direct callers holding the manager to invoke its factory methods directly:
class SK_API SkTypeface : public SkRefCnt {
#if !defined(SK_DISABLE_LEGACY_FONTMGR_REFDEFAULT)
    // Deprecated legacy factory relying on default singleton:
    static sk_sp<SkTypeface> MakeFromFile(const char path[], int index = 0);
#endif
};

// Caller invokes SkFontMgr directly:
sk_sp<SkTypeface> face = fontMgr->makeFromFile(path, index);
```
