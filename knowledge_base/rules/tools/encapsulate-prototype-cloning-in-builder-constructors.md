---
name: skia/tools/encapsulate-prototype-cloning-in-builder-constructors
description: When designing builder classes initialized from prototype objects, accept a const pointer or reference in the constructor and clone internally instead of taking std::unique_ptr
rubric: design
severity: should_fix
---

**Rule.** When designing a builder class that can be initialized from an
existing object or prototype, accept a const pointer or reference in the builder
constructor and clone or copy internally within member initialization. Do not
require callers or static Make() factories to pre-clone the object into a
std::unique_ptr parameter.

**Why.** Encapsulating object creation and cloning within the builder
constructor hides allocation and copying details from callers, simplifies call
sites by avoiding manual clone() invocations, and maintains clean value-oriented
construction semantics across builder hierarchies.

## Examples

Bad:

```cpp
class DisplayParamsBuilder {
public:
    // Requires factory or callers to manually clone and pass std::unique_ptr
    static std::unique_ptr<DisplayParamsBuilder> Make(const DisplayParams* other) {
        return std::unique_ptr<DisplayParamsBuilder>(new DisplayParamsBuilder(other->clone()));
    }
protected:
    DisplayParamsBuilder(std::unique_ptr<DisplayParams> other) : fDisplayParams(std::move(other)) {}
};
```

Good:

```cpp
class DisplayParamsBuilder {
public:
    // Builder constructor accepts borrowed pointer and handles cloning internally
    DisplayParamsBuilder(const DisplayParams* other)
            : fDisplayParams(other ? other->clone() : nullptr) {}
};
```
