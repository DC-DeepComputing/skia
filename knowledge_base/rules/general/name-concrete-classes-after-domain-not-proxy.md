---
name: skia/name-concrete-classes-after-domain-not-proxy
description: When defining concrete subclasses that inherit from proxy or adapter base classes, name them after the domain concept or platform rather than appending _proxy
rubric: readability
severity: should_fix
---

**Rule.** Do not encode internal delegation mechanisms (such as adding a
'_proxy' or 'Proxy' suffix) into concrete class names. Name concrete subclasses
after the domain concept, target platform, or backend they represent (e.g.,
'SkTypeface_fontconfig' rather than 'SkTypeface_fontconfig_proxy'), keeping
naming consistent with peer implementations.

**Why.** How a class implements its interface—such as delegating work through an
internal proxy or wrapper base class—is an implementation detail that should not
leak into class identity. Naming concrete subclasses after their domain role or
platform maintains clean encapsulation and naming parity across peer
implementations.

## Examples

Bad:

```cpp
// Bad: Leaking proxy delegation mechanics into the concrete platform typeface class name
class SkTypeface_fontconfig_proxy : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_fontconfig_proxy> Make(sk_sp<SkTypeface> realTypeface) {
        return sk_sp(new SkTypeface_fontconfig_proxy(std::move(realTypeface)));
    }
};
```

Good:

```cpp
// Good: Named after the platform backend concept, consistent with peer implementations
class SkTypeface_fontconfig : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_fontconfig> Make(sk_sp<SkTypeface> realTypeface) {
        return sk_sp(new SkTypeface_fontconfig(std::move(realTypeface)));
    }
};
```
