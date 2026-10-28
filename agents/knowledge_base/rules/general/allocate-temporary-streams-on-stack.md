---
name: skia/allocate-temporary-streams-on-stack
description: When instantiating temporary streams or stream adapters for local function calls, allocate them on the stack rather than on the heap with std::make_unique
rubric: safety
severity: should_fix
---

**Rule.** Allocate temporary streams and stream adapters (such as
SkMemoryStream, SkDynamicMemoryWStream, or rust::stream::SkStreamAdapter)
directly on the stack rather than on the heap with std::make_unique when their
lifetime is strictly local to a function call. Pass the stack-allocated stream
or adapter to callees by pointer or reference.

**Why.** Heap-allocating short-lived objects via std::make_unique introduces
unnecessary dynamic memory allocation overhead, heap fragmentation, and pointer
indirection. Stack allocation avoids allocator churn and preserves cache
locality during ephemeral operations like format sniffing and header parsing.

## Examples

Bad:

```cpp
// Bad: Unnecessary heap allocations for ephemeral stream and adapter
auto data = SkData::MakeWithoutCopy(buffer, bytesRead);
auto stream = std::make_unique<SkMemoryStream>(std::move(data));
auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
return rust_ico::is_ico(std::move(inputAdapter));
```

Good:

```cpp
// Good: Allocate stream and adapter on the stack and pass by pointer or reference
auto data = SkData::MakeWithoutCopy(buffer, bytesRead);
SkMemoryStream stream(std::move(data));
rust::stream::SkStreamAdapter inputAdapter(&stream);
return rust_ico::is_ico(inputAdapter);
```
