---
name: skia/assign-to-optional-directly
description: When assigning or initializing a std::optional<T> variable with a value or default instance, assign the value directly instead of wrapping it in redundant std::optional(...)
rubric: readability
severity: nit
---

**Rule.** Do not explicitly wrap values or default-constructed objects in
std::optional(...) or std::optional<T>(...) when assigning to or initializing a
std::optional<T> variable. Assign the value, temporary, or default-constructed
object directly, relying on std::optional's implicit converting assignment and
constructor.

**Why.** std::optional<T> provides converting constructors and assignment
operators that accept a value of type T (or types convertible to T) directly.
Wrapping the assigned expression in explicit std::optional constructor calls
adds visual noise, creates unnecessary temporaries, and obscures idiomatic
direct assignment.

## Examples

Bad:

```cpp
// Redundant explicit wrapping during assignment:
fBufferInfo = std::optional(bufferInfo);
fBufferInfo = std::optional(BindBufferInfo());

// Redundant wrapping during initialization:
std::optional<BindBufferInfo> opt = std::optional<BindBufferInfo>(bufferInfo);
```

Good:

```cpp
// Assign values or temporaries directly:
fBufferInfo = bufferInfo;
fBufferInfo = BindBufferInfo();

// Direct value initialization:
std::optional<BindBufferInfo> opt = bufferInfo;
```
