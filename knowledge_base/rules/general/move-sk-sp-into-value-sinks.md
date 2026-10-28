---
name: skia/move-sk-sp-into-value-sinks
description: When passing expiring movable objects or by-value sink parameters into storage, constructors, or containers, use std::move to avoid redundant copies and churn
rubric: safety
severity: should_fix
---

**Rule.** When passing an owned or expiring movable object (such as sk_sp<T>,
SkString, std::string, std::unique_ptr<T>, or containers) to a sink function,
constructor, or container method, or when storing a pass-by-value parameter into
a member or collection, wrap the argument in std::move. Do not pass, insert,
initialize, or assign movable types by lvalue copy when the source instance is
no longer needed.

**Why.** Copying an sk_sp<T> invokes SkSafeRef() and SkSafeUnref() causing
redundant atomic reference-count churn, while copying strings or containers
causes unnecessary heap allocations and buffer duplication. Moving transfers
ownership directly via pointer exchange or nullification with zero-copy
efficiency.

## Examples

Bad:

```cpp
// Bad: Passing, initializing, assigning, or inserting movable types by lvalue copy causes ref-count churn or heap copies
explicit SkFontStyleSet_AndroidNDK(sk_sp<TypefaceCache> cache)
    : fCache(cache) {}

void SetFactory(sk_sp<Factory> newFactory) {
    *get_factory() = newFactory;
}

std::unique_ptr<SkCodec> MakeCodec(sk_sp<SkData> data) {
    return std::unique_ptr<SkBmpRustCodec>(new SkBmpRustCodec(data));
}

void SlotManager::trackTextValue(SlotID id, sk_sp<TextAdapter> adapter) {
    fTextMap[id].push_back(adapter);
}

void SlotManager::setTextStringSlot(std::string slotID, SkString text) {
    fPropertyObserver->fTextMap[slotID] = text;
}
```

Good:

```cpp
// Good: std::move transfers ownership directly without atomic ref-count churn or redundant allocations
explicit SkFontStyleSet_AndroidNDK(sk_sp<TypefaceCache> cache)
    : fCache(std::move(cache)) {}

void SetFactory(sk_sp<Factory> newFactory) {
    *get_factory() = std::move(newFactory);
}

std::unique_ptr<SkCodec> MakeCodec(sk_sp<SkData> data) {
    return std::unique_ptr<SkBmpRustCodec>(new SkBmpRustCodec(std::move(data)));
}

void SlotManager::trackTextValue(SlotID id, sk_sp<TextAdapter> adapter) {
    fTextMap[id].push_back(std::move(adapter));
}

void SlotManager::setTextStringSlot(std::string slotID, SkString text) {
    fPropertyObserver->fTextMap[slotID] = std::move(text);
}
```
