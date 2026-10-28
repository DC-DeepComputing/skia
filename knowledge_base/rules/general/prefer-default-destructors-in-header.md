---
name: skia/prefer-default-destructors-in-header
description: When defining trivial, override, or virtual destructors without incomplete members, use = default inline in the class declaration instead of out-of-line empty definitions or = 0
rubric: readability
severity: nit
---

**Rule.** When a class destructor exhibits default destruction behavior and does
not require an out-of-line definition for incomplete forward-declared member
types (such as std::unique_ptr<T>), define it directly in the class declaration
using = default; (e.g., virtual ~Interface() = default; or ~Derived() override =
default;). Do not declare the destructor in the header and provide a redundant
empty {} body, a separate out-of-line defaulted definition, or a pure virtual
declaration (= 0) with an out-of-line empty definition in the .cpp file.

**Why.** Inlining = default; directly in the class declaration eliminates
boilerplate empty destructor definitions in .cpp files, signals trivial or
compiler-generated destruction semantics, and keeps class and interface
definitions concise. Making a destructor pure virtual (= 0) still requires an
out-of-line definition and adds unnecessary boilerplate when the interface
already contains pure virtual methods.

## Examples

Bad:

```cpp
// In header (.h):
class Agtm {
public:
    virtual ~Agtm() = 0;
};
class AgtmImpl final : public Agtm {
public:
    ~AgtmImpl() override;
};

// In implementation (.cpp):
Agtm::~Agtm() {}
AgtmImpl::~AgtmImpl() {}
```

Good:

```cpp
// In header (.h):
class Agtm {
public:
    virtual ~Agtm() = default;
};
class AgtmImpl final : public Agtm {
public:
    ~AgtmImpl() override = default;
};
```
