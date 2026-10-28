---
name: skia/api/avoid-subscript-zero-for-base-pointer
description: When retrieving the underlying buffer pointer from smart pointers or containers, use .get() or .data() instead of taking the address of element zero (&ptr[0])
rubric: correctness
severity: should_fix
---

**Rule.** Do not use `&ptr[0]` or `&container[0]` to retrieve the underlying
buffer pointer from smart pointers (such as `std::unique_ptr<T[]>`) or
contiguous containers. Use `.get()` or `.data()` instead to avoid undefined
behavior when the buffer is empty or the pointer is null.

**Why.** In C++, evaluating the subscript operator `operator[](0)` creates an
lvalue reference to element 0. When a smart pointer is null or a container is
empty, taking its address (`&ptr[0]`) dereferences a non-existent element, which
is undefined behavior. Member functions like `.get()` and `.data()` safely
return the underlying pointer (or `nullptr`) directly without indexing.

## Examples

Bad:

```cpp
// Undefined behavior when empty or null:
T* begin() {
    return &fData[0];
}

const uint8_t* bytes(const std::vector<uint8_t>& buf) {
    return &buf[0];
}
```

Good:

```cpp
// Safely returns nullptr or valid pointer without indexing:
T* begin() {
    return fData.get();
}

const uint8_t* bytes(const std::vector<uint8_t>& buf) {
    return buf.data();
}
```
