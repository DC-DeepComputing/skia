---
name: skia/omit-redundant-unique-ptr-upcast
description: When initializing or assigning a std::unique_ptr<Base> with std::make_unique<Derived>(), omit redundant explicit std::unique_ptr<Base>(...) wrapper casts and rely on implicit conversion
rubric: readability
severity: nit
---

**Rule.** Do not explicitly wrap std::make_unique<Derived>() in an outer
std::unique_ptr<Base>(...) constructor call when initializing, returning, or
assigning to a std::unique_ptr<Base> (including constructor member initializers
and function arguments). Rely instead on std::unique_ptr's implicit converting
move constructor to upcast derived instances to base pointer types.

**Why.** In C++, std::unique_ptr<Derived> implicitly converts to
std::unique_ptr<Base> via its converting constructor whenever Derived* is
implicitly convertible to Base*. Explicitly spelling out the outer
std::unique_ptr<Base>(...) wrapper introduces redundant type boilerplate and
visual noise without changing ownership or allocation semantics.

## Examples

Bad:

```cpp
// Redundant explicit wrapper in member initializer list:
fPendingDraws(std::unique_ptr<DrawListBase>(std::make_unique<DrawList>()))

// Redundant wrapper in assignment or return:
fState = std::unique_ptr<BaseState>(std::make_unique<DerivedState>(arg));
```

Good:

```cpp
// Rely on implicit unique_ptr converting constructor:
fPendingDraws(std::make_unique<DrawList>())

// Implicit conversion on assignment or return:
fState = std::make_unique<DerivedState>(arg);
```
