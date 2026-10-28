---
name: skia/api/dispatch-through-virtual-getter-not-member-field
description: When refactoring a class property to be virtual, update internal convenience helpers to invoke this->getter() instead of directly accessing raw member fields
rubric: correctness
severity: should_fix
---

**Rule.** When refactoring a property or field to be virtual (such as
introducing an NVI getter with onMethodName() hooks), update all convenience
methods and internal callers in the base class to dispatch through
this->getter() instead of reading the underlying member field directly.

**Why.** Reading member fields directly bypasses dynamic dispatch. Derived
classes and wrapper proxies that override or virtualize the property do not
update or synchronize the base member variable, causing base convenience helpers
to return stale or incorrect data.

## Examples

Bad:

```cpp
// Bad: fontStyle() is virtual/NVI, but helper methods read fStyle directly
SkFontStyle fontStyle() const { return this->onGetFontStyle(); }
bool isBold() const { return fStyle.weight() >= SkFontStyle::kSemiBold_Weight; }
```

Good:

```cpp
// Good: Convenience helper invokes the virtual getter to preserve dynamic dispatch
SkFontStyle fontStyle() const { return this->onGetFontStyle(); }
bool isBold() const { return this->fontStyle().weight() >= SkFontStyle::kSemiBold_Weight; }
```
