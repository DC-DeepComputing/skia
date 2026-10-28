---
name: skia/api/pass-lightweight-view-types-by-value
description: When declaring function parameters for lightweight view and span types (such as SkSpan, SkZip, or std::string_view), pass them by value rather than by reference
rubric: readability
severity: should_fix
---

**Rule.** Pass lightweight view and span types (such as SkSpan, SkZip,
std::span, std::string_view, or small non-owning domain wrappers like
UniformDataBlock) by value rather than by reference (const T& or T&). Do not
declare parameters taking cheap, trivially copyable non-owning views with const
or mutable reference wrappers.

**Why.** View and span types typically consist of only pointers and a size and
are designed to pass cheaply in CPU registers. Passing them by reference
introduces an unnecessary layer of pointer indirection, prevents compiler
optimizations, and misleadingly implies in-place mutation when using non-const
references.

## Examples

Bad:

```cpp
// Bad: passing lightweight view and span types by reference
void trackIntrinsic(const UniformDataBlock& intrinsicValues, uint32_t offset);
void processNames(const std::string_view& name, const SkSpan<const int>& values);
template <typename U>
SkSpan<const SkPoint> get_positions(SkZip<U, const SkPoint>& accepted);
```

Good:

```cpp
// Good: pass lightweight view and span types by value
void trackIntrinsic(UniformDataBlock intrinsicValues, uint32_t offset);
void processNames(std::string_view name, SkSpan<const int> values);
template <typename U>
SkSpan<const SkPoint> get_positions(SkZip<U, const SkPoint> accepted);
```
