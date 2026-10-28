---
name: skia/delete-copy-and-move-operations-together
description: When declaring, defaulting, or deleting copy or move constructors or assignment operators, explicitly declare, default, or delete all operations together (Rule of Five)
rubric: readability
severity: should_fix
---

**Rule.** When declaring, defaulting, or deleting any copy or move constructor
or assignment operator on a class or struct, explicitly declare, '= default', or
'= delete' the full set of copy and move operations (copy constructor, copy
assignment operator, move constructor, and move assignment operator) to adhere
to the Rule of Five. Do not delete only move operations to prevent copying, and
do not declare or default copy operations without move operations (or vice
versa), relying on implicit suppression or fallback.

**Why.** C++ special member function generation rules are interdependent and
easily misremembered: declaring user-defined or defaulted copy operations
suppresses implicit generation of move operations, causing moves to silently
fall back to expensive deep copies of container members. Conversely, declaring
or deleting move operations suppresses implicit copy operations, but relying on
implicit suppression is fragile and obscures whether non-copyability is
intentional.

## Examples

Bad:

```cpp
// Bad: deleting only move operations relies on implicit suppression of copy operations
class SkShaderBlitter : public SkRasterBlitter {
public:
    SkShaderBlitter(SkShaderBlitter&&) = delete;
    SkShaderBlitter& operator=(SkShaderBlitter&&) = delete;
};

// Bad: defaulting copy operations suppresses move operations, causing moves to deep-copy fCoordinates
class FontArguments {
public:
    FontArguments(const FontArguments& args) = default;
private:
    std::vector<SkFontArguments::VariationPosition::Coordinate> fCoordinates;
};
```

Good:

```cpp
// Good: explicitly delete both copy and move constructors and assignment operators
class SkShaderBlitter : public SkRasterBlitter {
public:
    SkShaderBlitter(const SkShaderBlitter&) = delete;
    SkShaderBlitter(SkShaderBlitter&&) = delete;
    SkShaderBlitter& operator=(const SkShaderBlitter&) = delete;
    SkShaderBlitter& operator=(SkShaderBlitter&&) = delete;
};

// Good: explicitly default both copy and move operations to preserve efficient moves
class FontArguments {
public:
    FontArguments(const FontArguments&) = default;
    FontArguments(FontArguments&&) = default;
    FontArguments& operator=(const FontArguments&) = default;
    FontArguments& operator=(FontArguments&&) = default;
private:
    std::vector<SkFontArguments::VariationPosition::Coordinate> fCoordinates;
};
```
