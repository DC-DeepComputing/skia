---
name: skia/api/provide-rvalue-accessors-on-result-wrappers
description: When defining value accessors on result, container, or wrapper types, provide rvalue-qualified overloads or move-out methods to avoid copying temporaries
rubric: safety
severity: should_fix
---

**Rule.** When defining value accessors on custom wrapper, container, or result
types, provide rvalue-qualified overloads (such as T&& value() && or T&&
operator*() &&) or explicit move-out methods (such as take()) alongside const&
accessors. Do not expose only const T& accessors when callers may need to
extract values from temporary or rvalue instances.

**Why.** Exposing only const lvalue accessors forces callers extracting data
from rvalues or temporaries to perform deep copies. For reference-counted
objects (such as sk_sp<T> or SkBitmap), this causes unnecessary atomic refcount
churn, and for move-only types (such as std::unique_ptr<T>), it prevents value
extraction entirely.

## Examples

Bad:

```cpp
template <typename T, typename E>
class SkResult {
public:
    // Bad: Only provides const lvalue accessor; callers cannot move out of temporaries.
    const T& value() const {
        SkASSERT(this->has_value());
        return std::get<T>(fStorage);
    }
};
```

Good:

```cpp
template <typename T, typename E>
class SkResult {
public:
    // Good: Provides both const& and && overloads to allow moving from temporaries.
    const T& value() const& {
        SkASSERT(this->has_value());
        return std::get<T>(fStorage);
    }
    T&& value() && {
        SkASSERT(this->has_value());
        return std::get<T>(std::move(fStorage));
    }
};
```
