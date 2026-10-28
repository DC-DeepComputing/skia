---
name: skia/api/preserve-typed-strings-across-intermediate-apis
description: When passing strings across intermediate layers or constructors, preserve typed string objects instead of prematurely decaying to raw C-strings
rubric: readability
severity: should_fix
---

**Rule.** Pass typed string objects (such as const SkString&, or const SkString*
when optional) across intermediate layers and constructors rather than decaying
to raw C-strings (const char* or const char[]) early. Convert to raw C-strings
or character pointers only at the leaf API that requires them.

**Why.** Passing typed string objects preserves type safety, length information,
and lifetime semantics across call stacks, and allows clean representation of
optional strings via pointers or std::optional. Prematurely converting to raw
C-strings (such as via .c_str()) degrades the abstraction and forces
intermediate layers to manage raw pointer lifetimes and re-measure string
lengths.

## Examples

Bad:

```cpp
// Intermediate class decays string to raw C-string array:
class SlotManagerWrapper {
public:
    explicit SlotManagerWrapper(const char path[]);
};

// Caller has an SkString and prematurely converts to const char*:
SkString fPath = get_path();
fSlotManagerWrapper = std::make_unique<SlotManagerWrapper>(fPath.c_str());
```

Good:

```cpp
// Intermediate class preserves typed string reference:
class SlotManagerWrapper {
public:
    explicit SlotManagerWrapper(const SkString& path);
    // Or if optional: explicit SlotManagerWrapper(const SkString* path = nullptr);
};

// Caller forwards typed object directly; only convert at leaf:
fSlotManagerWrapper = std::make_unique<SlotManagerWrapper>(fPath);
// In leaf implementation:
sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
```
