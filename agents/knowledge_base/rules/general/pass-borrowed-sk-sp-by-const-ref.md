---
name: skia/pass-borrowed-sk-sp-by-const-ref
description: When passing ref-counted smart pointers (sk_sp<T>) to functions that borrow, inspect, or forward without taking ownership, pass const sk_sp<T>& instead of by value
rubric: safety
severity: nit
---

**Rule.** When passing ref-counted smart pointers (sk_sp<T>) to functions that
only borrow, inspect, or forward the object without unconditionally taking
ownership, pass them by const reference (const sk_sp<T>&). Do not pass sk_sp<T>
by value (and never by const sk_sp<T>) unless the function unconditionally
consumes and assumes ownership of the instance.

**Why.** Copying an sk_sp<T> invokes atomic reference-count increments and
decrements, adding unnecessary synchronization overhead. Passing const sk_sp<T>
by value is especially wasteful because const values cannot be moved from;
passing const sk_sp<T>& avoids ref-count churn while allowing downstream
consumers to take an owning reference if needed.

## Examples

Bad:

```cpp
// Bad: Passing const sk_sp by value causes atomic ref churn and cannot be moved
Shaper::Result ShapeToFit(const SkString& txt,
                          const Shaper::TextDesc& orig_desc,
                          const SkRect& box,
                          const sk_sp<SkFontMgr>& fontmgr,
                          const sk_sp<SkShapers::Factory> shapingFactory);
```

Good:

```cpp
// Good: Passing borrowed sk_sp by const reference avoids atomic ref-count churn
Shaper::Result ShapeToFit(const SkString& txt,
                          const Shaper::TextDesc& orig_desc,
                          const SkRect& box,
                          const sk_sp<SkFontMgr>& fontmgr,
                          const sk_sp<SkShapers::Factory>& shapingFactory);
```
