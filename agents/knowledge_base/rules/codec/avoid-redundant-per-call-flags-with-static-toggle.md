---
name: skia/codec/avoid-redundant-per-call-flags-with-static-toggle
description: When defining factory or creation methods for classes with static configuration toggles, avoid redundant per-call override flags and query the static setting instead
rubric: design
severity: should_fix
---

**Rule.** Do not add redundant per-call configuration parameters (such as bool
forceX) to factory or creation methods when the enclosing class already provides
a static configuration toggle for that behavior. Expose the setting as a static
member function and let creation methods query that single source of truth
internally rather than accepting duplicate override arguments at call sites.

**Why.** Supplying both a static configuration toggle and a per-call override
parameter creates redundant configuration paths and unnecessary API complexity.
Centralizing the toggle as a static member function provides a single source of
truth and keeps factory call sites clean.

## Examples

Bad:

```cpp
void ForceSkcmsForICCProfiles(bool force);

class ICCProfileChromium {
public:
    static std::unique_ptr<ICCProfileChromium> Make(sk_sp<SkData> data, bool forceSkcms);
};
```

Good:

```cpp
class ICCProfileChromium {
public:
    static void ForceSkcms(bool forceSkcms);
    static std::unique_ptr<ICCProfileChromium> Make(sk_sp<SkData> data);
};
```
