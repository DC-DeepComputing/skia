---
name: skia/use-prepositional-qualifiers-for-attribute-methods
description: When naming methods that operate, measure capacity, or transform objects using a configured attribute, use a prepositional qualifier like With<Attribute> rather than a bare noun
rubric: readability
severity: should_fix
---

**Rule.** When naming methods that operate, measure capacity, or transform
objects according to an established attribute or secondary configuration, use a
prepositional qualifier (such as WithStride, WithFilter, or WithMatrix) rather
than concatenating the attribute noun directly to the verb or property name
(e.g., use appendMappedWithStride instead of appendMappedStride,
availableWithStride instead of availableStride).

**Why.** Directly suffixing an attribute noun (like availableStride or
appendMappedStride) makes the attribute appear to be the direct object of the
verb or the value being queried, implying the method returns or appends the
stride itself. Adding a preposition like With... clarifies that the operation or
capacity measurement operates on element counts using the established stride or
modifier.

## Examples

Bad:

```cpp
// Ambiguous: implies appending a stride or returning the stride itself.
BufferWriter appendMappedStride(size_t count);
uint32_t availableStride() const;
```

Good:

```cpp
// Clear: preposition clarifies the operation uses the configured stride.
BufferWriter appendMappedWithStride(size_t count);
uint32_t availableWithStride() const;
```
