---
name: skia/store-enums-as-const-constructor-initialized-members
description: When storing immutable configuration modes in classes, preserve strongly typed enums instead of booleans and initialize them as const constructor members
rubric: readability
severity: nit
---

**Rule.** Store strongly typed enums rather than boolean flags for configuration
modes, and pass them into the constructor to initialize immutable member
variables as const. Do not collapse enums into booleans or mutate internal
configuration fields post-construction after factory instantiation.

**Why.** Collapsing an enum into a boolean loses type semantics and prevents
future mode additions. Initializing configuration members in the constructor
allows them to be declared const, preventing accidental mutation and avoiding
the exposure of mutable internal fields.

## Examples

Bad:

```cpp
// Bad: collapsing an enum into a boolean and mutating after construction
class SkBmpRustCodec : public SkCodec {
    ...
    bool fIsFromIco = false;
};

auto codec = MakeFromReader(std::move(stream), std::move(reader), result);
if (codec) {
    codec->fIsFromIco = (type == StreamType::kICO);
}
```

Good:

```cpp
// Good: storing the enum as a const member initialized in the constructor
class SkBmpRustCodec : public SkCodec {
    SkBmpRustCodec(..., StreamType streamType)
        : ...
        , fStreamType(streamType) {}
    ...
    const StreamType fStreamType;
};

auto codec = MakeFromReader(std::move(stream), std::move(reader), type, result);
```
