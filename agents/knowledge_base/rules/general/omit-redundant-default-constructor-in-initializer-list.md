---
name: skia/omit-redundant-default-constructor-in-initializer-list
description: When writing constructor member initializer lists, omit redundant parameterless default constructor calls for base classes and default-constructible member variables
rubric: readability
severity: nit
---

**Rule.** Do not explicitly invoke default constructors for base classes or
default-constructible member variables in constructor member initializer lists
when calling them with no arguments (e.g., : BaseClass(), , fMember(), or ,
fMember{}). Rely on standard C++ implicit default-initialization to construct
base subobjects and members.

**Why.** In C++, base classes and member variables with default constructors are
automatically default-initialized in declaration order if omitted from the
constructor initializer list. Explicitly invoking their parameterless
constructors adds visual noise and redundancy without altering runtime
initialization semantics.

## Examples

Bad:

```cpp
// Bad: Redundant explicit default constructor calls for base class and member
SkPixelRef::SkPixelRef(int width, int height, void* pixels, size_t rowBytes)
        : SkPixelStorage()
        , fWidth(width)
        , fHeight(height)
        , fPixels(pixels)
        , fGenerationID() {}
```

Good:

```cpp
// Good: Omit redundant parameterless default constructor calls
SkPixelRef::SkPixelRef(int width, int height, void* pixels, size_t rowBytes)
        : fWidth(width)
        , fHeight(height)
        , fPixels(pixels) {}
```
