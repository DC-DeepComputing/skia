---
name: skia/avoid-move-on-view-types
description: When passing or forwarding lightweight view types like std::string_view or SkSpan, pass them directly by value instead of calling std::move
rubric: readability
severity: should_fix
---

**Rule.** Do not call std::move() when passing, returning, or forwarding
lightweight view and span types (such as std::string_view or SkSpan). Pass them
directly by value without wrapping them in std::move().

**Why.** View types like std::string_view and SkSpan are cheap, trivially
copyable structures consisting only of a pointer and a size. Calling std::move()
on them is an ineffective no-op that adds visual clutter and misleadingly
implies resource transfer or invalidation of the source.

## Examples

Bad:

```cpp
sk_sp<DawnBuffer> DawnBuffer::Make(const DawnSharedContext* ctx,
                                   size_t size,
                                   std::string_view label) {
    return sk_sp<DawnBuffer>(new DawnBuffer(ctx, size, std::move(label)));
}
```

Good:

```cpp
sk_sp<DawnBuffer> DawnBuffer::Make(const DawnSharedContext* ctx,
                                   size_t size,
                                   std::string_view label) {
    return sk_sp<DawnBuffer>(new DawnBuffer(ctx, size, label));
}
```
