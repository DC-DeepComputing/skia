---
name: skia/define-cpp-helper-methods-in-declaration
description: When declaring internal helper or nested implementation classes in C++ source files, define member functions inline within the class declaration instead of out-of-line
rubric: readability
severity: should_fix
---

**Rule.** When declaring internal helper classes or nested implementation
classes scoped to a C++ source file (.cpp), define member functions
(particularly constructors, destructors, and short helper methods) directly
inside the class declaration. Do not separate member declarations and
definitions using redundant out-of-line qualifiers (e.g.,
OuterClass::InnerClass::method()) within the translation unit.

**Why.** Helper classes defined locally within a .cpp file have no external
callers or consumers and do not need separate interface definitions. Defining
member functions inline within the class declaration keeps the helper cohesive
and readable while eliminating boilerplate and verbose nested scope qualifiers.

## Examples

Bad:

```cpp
class DawnResourceProvider::IntrinsicConstantsManager {
public:
    explicit IntrinsicConstantsManager(DawnResourceProvider* resourceProvider);
    ~IntrinsicConstantsManager();

    void purgeBuffers();
};

DawnResourceProvider::IntrinsicConstantsManager::~IntrinsicConstantsManager() {
    this->purgeBuffers();
    SkASSERT(fIntrinsicBuffersLRU.isEmpty());
}
```

Good:

```cpp
class DawnResourceProvider::IntrinsicConstantsManager {
public:
    explicit IntrinsicConstantsManager(DawnResourceProvider* resourceProvider)
            : fResourceProvider(resourceProvider) {}
    ~IntrinsicConstantsManager() {
        this->purgeBuffers();
        SkASSERT(fIntrinsicBuffersLRU.isEmpty());
    }

    void purgeBuffers();
};
```
