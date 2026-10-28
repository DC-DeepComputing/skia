---
name: skia/avoid-const-on-abstract-interface-methods
description: When declaring virtual methods in abstract interfaces or factories, avoid const qualifiers unless strictly required, preserving derived implementation flexibility for caching or mutation
rubric: readability
severity: should_fix
---

**Rule.** Do not mark virtual methods in abstract interfaces or polymorphic base
classes (such as factories, providers, or builders) as const unless strictly
required. Keep interface methods non-const to allow derived implementations to
update caches, perform lazy initialization, or invoke non-const methods on
internal member fields without resorting to mutable members or const_cast.

**Why.** Marking abstract interface methods const imposes const constraints on
all future subclasses. Derived implementations often need to lazily initialize
resources, cache intermediate calculations, or invoke non-const methods on
member fields, which would otherwise force subclasses to use mutable fields or
unsafe const_casts.

## Examples

Bad:

```cpp
// BAD: Marking abstract interface methods const constrains implementations from caching or lazy-init.
class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) const = 0;
    virtual SkUnicode* getUnicode() const = 0;
};
```

Good:

```cpp
// GOOD: Leave virtual interface and factory methods non-const so subclasses have implementation flexibility.
class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) = 0;
    virtual SkUnicode* getUnicode() = 0;
};
```
