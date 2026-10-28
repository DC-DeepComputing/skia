---
name: skia/gpu/accept-resource-directly-over-binding-descriptor
description: When methods or helpers inherently require an entire GPU resource, accept the resource handle directly rather than an open-ended BindBufferInfo descriptor
rubric: design
severity: should_fix
---

**Rule.** Do not require callers to pass open-ended view or binding descriptors
(such as BindBufferInfo) when a method inherently expects the entire resource.
Accept the resource handle directly (such as sk_sp<Buffer> or const Buffer*) and
construct the full-resource binding descriptor internally with a zero offset and
complete size.

**Why.** Accepting slice or binding structs (which pair a resource pointer with
byte offsets and sizes) when an operation requires the full resource exposes
unnecessary flexibility that callers can misuse by passing invalid offsets or
sub-ranges. Accepting the resource directly simplifies call sites and guarantees
the full-resource invariant.

## Examples

Bad:

```cpp
// Bad: Exposes unnecessary offset and size parameters when the full buffer is required.
class GlobalCache {
public:
    void addStaticVertexBuffer(const BindBufferInfo& bufferInfo, sk_sp<Resource> resource);
};
```

Good:

```cpp
// Good: Accepts the resource directly and constructs the binding descriptor internally.
class GlobalCache {
public:
    void setStaticVertexBuffer(sk_sp<Buffer> buffer) {
        fStaticVertexData = {buffer.get(), /*offset=*/0, buffer->size()};
        this->addStaticResource(std::move(buffer));
    }
};
```
