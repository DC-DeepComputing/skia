---
name: skia/api/when-declaring-immutable-member-dependencies-such-as-sk-sp-s
description: When declaring immutable member dependencies such as sk_sp smart pointers fixed at construction, mark them const and group related fields together
rubric: readability
severity: should_fix
---

**Rule.** Declare immutable member variables—including dependency smart pointers
like sk_sp<T>—as const when their values are fixed at construction and never
rebound, consistent with surrounding immutable members. In addition, group
logically related members (such as font management and text shaping
dependencies) together in class definitions.

**Why.** Marking immutable member dependencies as const prevents accidental
reassignment, communicates immutability invariants to readers and the compiler,
and preserves consistency across class definitions. Grouping related members
together clarifies dependencies and improves class layout readability.

## Examples

Bad:

```cpp
// Member left mutable despite being fixed at construction:
class SlotManager final : public skottie::SlotManager {
    ...
    sk_sp<skottie::internal::SceneGraphRevalidator> fRevalidator;
};

// Mixed constness and ungrouped related dependencies:
class SkSVGDOM : public SkRefCnt {
    const sk_sp<SkSVGSVG>                       fRoot;
    const sk_sp<SkFontMgr>                      fFontMgr;
    const sk_sp<skresources::ResourceProvider>  fResourceProvider;
    sk_sp<SkShapers::Factory>                   fTextShapingFactory;
};
```

Good:

```cpp
// Declared const when initialized at construction and never rebound:
class SlotManager final : public skottie::SlotManager {
    ...
    const sk_sp<skottie::internal::SceneGraphRevalidator> fRevalidator;
};

// Const-qualified and logically grouped dependencies:
class SkSVGDOM : public SkRefCnt {
    const sk_sp<SkSVGSVG>                       fRoot;
    const sk_sp<SkFontMgr>                      fFontMgr;
    const sk_sp<SkShapers::Factory>             fTextShapingFactory;
    const sk_sp<skresources::ResourceProvider>  fResourceProvider;
};
```
