---
name: skia/document-callable-template-signatures
description: When defining templated visitor, iteration, or callback functions taking generic callable parameters like template <typename Fn>, document the expected signature in an inline comment
rubric: readability
severity: should_fix
---

**Rule.** When declaring templated functions or methods that accept an
unconstrained callable parameter (such as template <typename Fn>), document the
expected callable signature (e.g., // (Task*)->Status or // f(T*)) in an inline
comment. Do not leave the callable signature unspecified or require callers to
inspect the function body to discover required arguments and return types.

**Why.** Unconstrained template callable parameters (like Fn) do not communicate
parameter types or return values in C++ function signatures. Documenting the
expected callable contract directly in a comment clarifies the interface for
callers and prevents compile-time errors from mismatched arguments.

## Examples

Bad:

```cpp
template <typename Fn>
Task::Status visitTasks(Fn);

template <typename Fn>
void foreach(Fn);
```

Good:

```cpp
template <typename Fn> // (Task*)->Status
Task::Status visitTasks(Fn);

template <typename Fn> // f(T*) or f(const T&)
void foreach(Fn);
```
