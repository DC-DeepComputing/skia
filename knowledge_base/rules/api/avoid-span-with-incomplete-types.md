---
name: skia/api/avoid-span-with-incomplete-types
description: When declaring span members in recursive or forward-declared structs, do not instantiate std::span or SkSpan with incomplete types; store raw pointers or dedicated containers
rubric: correctness
severity: blocking
---

**Rule.** Do not instantiate `std::span<T>` or `SkSpan<T>` with an incomplete
element type `T`, such as declaring a span member referencing the enclosing
struct within its own definition or using forward-declared types. In recursive
or self-referential structures, store raw pointer and count pairs (e.g., `T*
fChildren` and `size_t fChildCount`) or container types specifically designed to
accommodate incomplete types.

**Why.** Per C++ standard [span.overview], `std::span` requires `ElementType` to
be a complete object type that is not an abstract class type. Instantiating
`std::span` (or `SkSpan` when aliased to `std::span`) with an incomplete type is
undefined behavior and triggers compilation errors across standard library
implementations (such as libc++). Workarounds like deleting copy constructors do
not eliminate the standard violation.

## Examples

Bad:

```cpp
struct TreeNode {
    TreeNode* fParent = nullptr;
    // Undefined behavior: TreeNode is incomplete at member declaration
    std::span<TreeNode> fChildren;
};
```

Good:

```cpp
struct TreeNode {
    TreeNode* fParent = nullptr;
    // Safe: raw pointer and count accommodate incomplete types
    TreeNode* fChildren = nullptr;
    size_t fChildCount = 0;
};
```
