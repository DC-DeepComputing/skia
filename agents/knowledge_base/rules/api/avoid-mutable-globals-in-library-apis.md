---
name: skia/api/avoid-mutable-globals-in-library-apis
description: When designing library APIs or configuring subsystems, avoid mutable global state and process-wide setters; inject dependencies and factories explicitly via builder or instance objects
rubric: design
severity: should_fix
---

**Rule.** Do not expose mutable globals, static variables, or process-wide
configuration setters in library APIs. Pass dependencies, factories, and
configuration options explicitly to builder methods, constructors, or instance
objects to keep libraries modular and re-entrant.

**Why.** Mutable global state prevents concurrent use across multiple threads
and causes unpredictable cross-module side effects. It also breaks embedding
environments where multiple independent clients or components in the same
application process require different configurations or backend implementations.

## Examples

Bad:

```cpp
// Bad: process-wide mutable global setter in library API
static sk_sp<SkShapers::Factory>* get_factory() {
    static sk_sp<SkShapers::Factory> factory = SkShapers::Primitive::Factory();
    return &factory;
}

void SetFactory(sk_sp<SkShapers::Factory> newFactory) {
    *get_factory() = std::move(newFactory);
}
```

Good:

```cpp
// Good: inject dependencies explicitly via instance or builder methods
class Builder {
public:
    Builder& setTextShapingFactory(sk_sp<SkShapers::Factory> factory) {
        fShapingFactory = std::move(factory);
        return *this;
    }
private:
    sk_sp<SkShapers::Factory> fShapingFactory;
};
```
