---
name: skia/api/delegate-convenience-interface-overloads
description: When designing interface overloads that supply default parameter values, avoid redundant pure virtual methods; provide non-virtual delegating base implementations or require explicit arguments
rubric: design
severity: should_fix
---

**Rule.** When designing or extending an interface or base class with
convenience overloads that supply default parameter values, do not declare
multiple pure virtual overloads (= 0). Instead, keep the virtual interface
minimal: either provide an inline non-virtual delegating method in the base
class that forwards to the primary generalized virtual method, or require
callers to pass the parameter explicitly.

**Why.** Declaring redundant pure virtual overloads forces every derived
subclass to implement boilerplate forwarding methods, duplicating trivial
parameter passing across implementations and creating maintenance churn.
Centralizing default argument forwarding in a non-virtual base helper or keeping
the interface minimal ensures subclasses only override canonical behavior.

## Examples

Bad:

```cpp
// BAD: Multiple pure virtual overloads forcing subclasses to write forwarding boilerplate.
class SkExecutor {
public:
    virtual void add(std::function<void()>) = 0;
    virtual void add(std::function<void()>, int workList) = 0;
};

class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<RunIter> make(const char* text, size_t len) const = 0;
    virtual std::unique_ptr<RunIter> make(const char* text, size_t len, Tag tag) const = 0;
};
```

Good:

```cpp
// GOOD: Provide non-virtual delegating helpers or expose only the generalized virtual method.
class SkExecutor {
public:
    void add(std::function<void()> fn) { this->add(std::move(fn), 0); }
    virtual void add(std::function<void()>, int workList) = 0;
};

class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<RunIter> make(const char* text, size_t len, Tag tag) const = 0;
};
```
