---
name: skia/api/prefer-interface-over-callback-tuple
description: When designing APIs that take multiple related callbacks, prefer an abstract SkRefCnt interface class passed via sk_sp over std::tuple of std::function
rubric: readability
severity: should_fix
---

**Rule.** When an API requires multiple interrelated callbacks or factories,
prefer defining an abstract interface class (deriving from SkRefCnt and managed
via sk_sp) with pure virtual methods rather than bundling multiple std::function
objects in a std::tuple passed by value.

**Why.** A std::tuple of std::function objects has a large memory footprint due
to small-buffer optimization and can trigger dynamic heap allocations when
copying captured lambda state. An abstract SkRefCnt interface passed via sk_sp
enables cheap refcounted pointer sharing and cleanly encapsulates shared state
or lifecycle across interrelated methods.

## Examples

Bad:

```cpp
// Heavyweight: multiple std::function objects in a tuple copied by value
using Callbacks = std::tuple<
    std::function<std::unique_ptr<SkShaper>(sk_sp<SkFontMgr>)>,
    std::function<std::unique_ptr<SkShaper::BiDiRunIterator>(const char*, size_t)>
>;
Builder& setTextShapingCallbacks(Callbacks callbacks);
```

Good:

```cpp
// Lightweight: virtual interface encapsulates callbacks and shared state via sk_sp
class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const = 0;
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
            const char* utf8, size_t utf8Bytes) const = 0;
};
Builder& setTextShapingFactory(sk_sp<Factory> factory);
```
