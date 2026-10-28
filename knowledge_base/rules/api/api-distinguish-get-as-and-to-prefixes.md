---
name: skia/api/api-distinguish-get-as-and-to-prefixes
description: When naming methods that access resources, views, or conversions, reserve get* for properties or buffers, as* for polymorphic views, and to* for transformations
rubric: readability
severity: should_fix
---

**Rule.** Name accessors that retrieve an underlying member, buffer, or owned
resource using getter prefixes (e.g., getData()) rather than as... or to....
Reserve as... (e.g., asBlendMode(), asDirectContext()) for treating or viewing
the object itself as an alternate type or interface without auxiliary state, and
reserve to... (e.g., toQuad(), toSkColor()) for value conversions or format
transformations.

**Why.** Naming an accessor as... misleadingly implies that the object itself is
being viewed as an alternate interface; this is inaccurate when the host object
maintains independent state (such as a stream maintaining a read offset over
underlying data). Similarly, to... implies an active conversion or
transformation rather than accessing an existing backing resource.
Distinguishing these prefixes maintains predictable API semantics across Skia.

## Examples

Bad:

```cpp
class SkStream {
public:
    // Bad: Stream maintains its own read offset, so accessing the underlying
    // buffer is not a polymorphic view ('as') or conversion ('to').
    virtual sk_sp<const SkData> asData() const { return nullptr; }
    virtual sk_sp<const SkData> toData() const { return nullptr; }
};
```

Good:

```cpp
class SkStream {
public:
    // Good: Use get* for accessors retrieving underlying resources or buffers.
    virtual sk_sp<const SkData> getData() const { return nullptr; }
};

class SkPaint {
public:
    // Good: as* views the object itself as an alternate representation.
    std::optional<SkBlendMode> asBlendMode() const;
};

class SkColor4f {
public:
    // Good: to* performs a value conversion.
    SkColor toSkColor() const;
};
```
