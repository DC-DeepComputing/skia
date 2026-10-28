---
name: skia/document-default-serial-procs-assumptions
description: When initializing serialization buffers with default SkSerialProcs or SkDeserialProcs, add a comment documenting the invariant ensuring unhandled types like SkImage are absent
rubric: readability
severity: nit
---

**Rule.** When initializing serialization buffers (such as SkBinaryWriteBuffer,
SkWriteBuffer, or SkReadBuffer) or invoking serialization methods with empty or
default SkSerialProcs / SkDeserialProcs, add an inline comment documenting why
default procs suffice. Explicitly state the invariant or domain guarantee that
types requiring custom handlers (such as SkImage, SkTypeface, or SkPicture) will
not be encountered.

**Why.** Default SkSerialProcs and SkDeserialProcs cannot serialize or
deserialize types like SkImage or SkTypeface that require dedicated external
procs. Relying on default procs introduces an implicit cross-call invariant;
documenting this assumption ensures future maintainers understand why custom
procs are omitted and prevents silent serialization failures or data drops when
data formats evolve.

## Examples

Bad:

```cpp
// Writing buffer without documenting why default procs are safe:
SkBinaryWriteBuffer writeBuffer{nullptr, 0, {}};

// Reading buffer without documenting proc assumptions:
SkReadBuffer readBuffer{data->data(), data->size()};
```

Good:

```cpp
// We can use default SkSerialProcs because glyph data does not encode SkImages.
SkBinaryWriteBuffer writeBuffer{nullptr, 0, {}};

// Default SkDeserialProcs suffice because writeStrikeData did not encode SkImages.
SkReadBuffer readBuffer{data->data(), data->size()};
```
