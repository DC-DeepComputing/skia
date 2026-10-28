---
name: skia/api/hoist-deleted-copy-move-to-base-class
description: When making a class hierarchy non-copyable or non-movable, hoist deleted copy and move operations to the base class instead of duplicating them across subclasses
rubric: design
severity: should_fix
---

**Rule.** When designing or extending a class hierarchy where all derived types
should be non-copyable or non-movable, declare the copy and move operations as
deleted (`= delete`) in the common base class instead of repeating deleted
operations across individual subclasses. Derived classes implicitly inherit the
suppression of copy and move operations from a non-copyable or non-movable base
class.

**Why.** Declaring copy and move constructors and assignment operators as
deleted in a base class prevents the compiler from generating copy and move
operations for all derived classes. Repeating `= delete` in every subclass adds
unnecessary boilerplate and risks omissions when new subclasses are added.

## Examples

Bad:

```cpp
// Duplicating deleted operations across every derived class:
class SkShaderBlitter : public SkRasterBlitter {
public:
    SkShaderBlitter(const SkShaderBlitter&) = delete;
    SkShaderBlitter(SkShaderBlitter&&) = delete;
    SkShaderBlitter& operator=(const SkShaderBlitter&) = delete;
    SkShaderBlitter& operator=(SkShaderBlitter&&) = delete;
};
```

Good:

```cpp
// Hoisted to the base class; derived classes inherit non-copyable semantics:
class SkBlitter {
public:
    SkBlitter(const SkBlitter&) = delete;
    SkBlitter(SkBlitter&&) = delete;
    SkBlitter& operator=(const SkBlitter&) = delete;
    SkBlitter& operator=(SkBlitter&&) = delete;
};

class SkShaderBlitter : public SkBlitter {
    // Subclasses automatically have copy and move operations suppressed.
};
```
