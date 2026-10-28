---
name: skia/prefer-base-qualification-over-casting-this
description: When invoking base class methods from a derived class, use qualified lookup like this->Base::method() instead of casting this with static_cast
rubric: readability
severity: nit
---

**Rule.** When delegating to or invoking a base class member function from a
derived class (including reusing a base const method to implement a non-const
overload), invoke the base method using qualified member lookup
(`this->Base::method(...)` or `Base::method(...)`). Do not cast `this` to a base
class pointer or reference (such as `static_cast<const
Base*>(this)->method(...)`).

**Why.** In derived classes, base class methods are directly accessible;
qualified lookup (`this->Base::method(...)`) cleanly directs overload resolution
and suppresses virtual dispatch without unnecessary cast syntax. Conversely,
casting `this` via `static_cast<Base*>(this)` adds verbose boilerplate, and if
the method is virtual, casting `this` still triggers dynamic dispatch back to
the derived override rather than invoking the base implementation.

## Examples

Bad:

```cpp
// Bad: casting 'this' to a base pointer to delegate or invoke base methods
uint8_t* SkMaskBuilder::getAddr1(int x, int y) {
    return const_cast<uint8_t*>(static_cast<const SkMask*>(this)->getAddr1(x, y));
}

void Derived::draw() {
    static_cast<Base*>(this)->draw();
}
```

Good:

```cpp
// Good: use qualified base member lookup directly on 'this'
uint8_t* SkMaskBuilder::getAddr1(int x, int y) {
    return const_cast<uint8_t*>(this->SkMask::getAddr1(x, y));
}

void Derived::draw() {
    this->Base::draw();
}
```
