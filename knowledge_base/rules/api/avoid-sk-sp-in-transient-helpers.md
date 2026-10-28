---
name: skia/api/avoid-sk-sp-in-transient-helpers
description: When defining transient helper structs or traversal contexts, borrow non-owning raw pointers (const T*) or const references (const sk_sp<T>&) instead of storing sk_sp<T> by value
rubric: safety
severity: should_fix
---

**Rule.** Do not store ref-counted smart pointers by value (sk_sp<T>) as member
variables in transient, short-lived helper structs, iterators, or
stack-allocated traversal contexts when the referenced object outlives the
operation. Borrow a non-owning raw pointer (const T* or T*) instead, or pass and
hold a const reference (const sk_sp<T>&) when existing interfaces or accessors
require the smart-pointer handle.

**Why.** Transient helper and traversal contexts (such as SkDraw or
SkSVGRenderContext) do not own the contexts, devices, shapers, or font managers
they orchestrate, and are frequently copied by value during tree traversal or
draw iteration. Storing sk_sp<T> by value incurs redundant atomic
reference-count increments and decrements on construction and copying, and
forces full header inclusions where forward declarations or borrowed references
suffice.

## Examples

Bad:

```cpp
// Bad: transient helper and traversal structs store owning sk_sp by value,
// causing atomic ref-count churn on construction and tree-traversal copies.
class SkDrawBase {
public:
    sk_sp<const SkRasterContextPriv> fCtx{nullptr};
};

class SkSVGRenderContext {
public:
    SkSVGRenderContext(..., sk_sp<SkShapers::Factory> factory);

    sk_sp<SkShapers::Factory> fTextShapingFactory;
};
```

Good:

```cpp
// Good: borrow non-owning raw pointers with forward declarations, or hold
// const sk_sp<T>& references for stack-scoped traversal contexts.
class SkRasterContext_Base;
class SkDrawBase {
public:
    const SkRasterContext_Base* fCtx{nullptr};
};

class SkSVGRenderContext {
public:
    SkSVGRenderContext(..., const sk_sp<SkShapers::Factory>& factory);

    const sk_sp<SkShapers::Factory>& fTextShapingFactory;
};
```
