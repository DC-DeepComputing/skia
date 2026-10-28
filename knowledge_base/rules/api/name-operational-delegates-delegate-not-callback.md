---
name: skia/api/name-operational-delegates-delegate-not-callback
description: When defining callables or parameters that execute an operation, name them Delegate instead of Callback, reserving Callback for event notifications and hooks
rubric: readability
severity: should_fix
---

**Rule.** Name callable type aliases, interfaces, and function parameters that
are delegated the responsibility of executing an active operation (such as
drawing, transforming, or processing) 'Delegate' or '*Delegate' (and 'delegate'
for parameter and variable names) rather than '*Callback'. Reserve 'Callback'
for event listeners, lifecycle observers, visitor hooks, and completion
notifications.

**Why.** The term 'Callback' conveys an event listener or notification hook
reacting to an event. When an operation delegates its core execution
responsibility to a caller-supplied function or interface, naming it 'Delegate'
accurately conveys the delegation design pattern and prevents confusion about
whether the callable merely observes an event or actively performs the
operation.

## Examples

Bad:

```cpp
// Misleading: active draw operation is labeled as a callback
using AtlasDrawCallback = std::function<void(const sktext::gpu::AtlasSubRun* subRun,
                                             SkPoint drawOrigin,
                                             const SkPaint& paint,
                                             sk_sp<SkRefCnt> subRunStorage)>;

void draw(SkCanvas*, SkPoint drawOrigin, const SkPaint&, const SkRefCnt* subRunStorage,
          AtlasDrawCallback atlasCallback) const;
```

Good:

```cpp
// Clear: Delegate conveys delegated operational execution
using AtlasDrawDelegate = std::function<void(const sktext::gpu::AtlasSubRun* subRun,
                                             SkPoint drawOrigin,
                                             const SkPaint& paint,
                                             sk_sp<SkRefCnt> subRunStorage)>;

void draw(SkCanvas*, SkPoint drawOrigin, const SkPaint&, const SkRefCnt* subRunStorage,
          AtlasDrawDelegate atlasDelegate) const;
```
