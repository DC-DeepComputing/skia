---
name: skia/omit-unnecessary-default-constructors
description: When defining classes requiring specific dependencies or configuration, require them in constructors rather than exposing default constructors or post-construction setters
rubric: readability
severity: should_fix
---

**Rule.** Do not define parameterless default constructors or rely on
post-construction setters for classes, buffers, or internal helpers when
instances require specific dependencies, external configuration, or established
invariants to function correctly. Require callers to supply all necessary
dependencies and configuration (such as SkSerialProcs or asset pointers) at
construction time—passing explicit empty values (such as `{}`) where
optional—rather than exposing an unconfigured, incomplete, or invalid state.

**Why.** Omitting dependencies or relying on post-construction setters leaves
objects in an uninitialized or unconfigured state where subsequent operations
can fail unexpectedly (such as failing to serialize images). Enforcing complete
initialization in constructors guarantees valid invariants at compile time and
eliminates dead or dangerous default states.

## Examples

Bad:

```cpp
// Bad: Exposing an unused or invalid default constructor
class ImageAssetProxy final : public skresources::ImageAsset {
public:
    ImageAssetProxy() {}
    explicit ImageAssetProxy(sk_sp<skresources::ImageAsset> asset)
        : fImageAsset(std::move(asset)) {}
};

// Bad: Relying on post-construction setters for mandatory configuration
SkBinaryWriteBuffer writeBuffer{buffer, size};
writeBuffer.setSerialProcs(procs);
```

Good:

```cpp
// Good: Require mandatory dependencies at construction time
class ImageAssetProxy final : public skresources::ImageAsset {
public:
    explicit ImageAssetProxy(sk_sp<skresources::ImageAsset> asset)
        : fImageAsset(std::move(asset)) {}
};

// Good: Pass configuration directly to constructor; pass {} if empty
SkBinaryWriteBuffer writeBuffer{buffer, size, procs};
```
