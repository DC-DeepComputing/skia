---
name: skia/pass-non-owning-sk-sp-by-const-ref
description: When passing or holding ref-counted objects (sk_sp<T>) in non-owning functions or ephemeral stack-scoped contexts, use const sk_sp<T>& instead of copying by value
rubric: safety
severity: blocking
---

**Rule.** When declaring function or constructor parameters and member variables
for ref-counted objects (sk_sp<T>) in functions or ephemeral, stack-scoped
context objects that borrow without taking ownership, pass and store them as
const sk_sp<T>& rather than by value. Reserve passing sk_sp<T> by value (or
transferring via std::move) for functions, constructors, or sinks that genuinely
take ownership into persistent storage.

**Why.** Passing or storing sk_sp<T> by value invokes SkSafeRef() and
SkSafeUnref(), causing redundant atomic reference-count increments and
decrements whenever functions are called or stack-scoped context structs are
instantiated and copied across tree traversals. Passing and holding const
sk_sp<T>& borrows the reference with zero atomic churn while keeping the
underlying handle accessible.

## Examples

Bad:

```cpp
// Bad: Passing or storing sk_sp by value when borrowing causes redundant atomic refcount churn
void Process(sk_sp<SkFontMgr> fontMgr, sk_sp<SkShapers::Factory> factory);

class SkSVGRenderContext {
public:
    SkSVGRenderContext(SkCanvas*,
                       const sk_sp<SkFontMgr>&,
                       sk_sp<SkShapers::Factory>);
    ...
private:
    sk_sp<SkShapers::Factory> fTextShapingFactory;
};
```

Good:

```cpp
// Good: Pass and hold const sk_sp<T>& when non-owning contexts borrow references
void Process(const sk_sp<SkFontMgr>& fontMgr, const sk_sp<SkShapers::Factory>& factory);

class SkSVGRenderContext {
public:
    SkSVGRenderContext(SkCanvas*,
                       const sk_sp<SkFontMgr>&,
                       const sk_sp<SkShapers::Factory>&);
    ...
private:
    const sk_sp<SkShapers::Factory>& fTextShapingFactory;
};
```
