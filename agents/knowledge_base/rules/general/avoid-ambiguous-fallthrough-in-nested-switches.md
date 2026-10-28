---
name: skia/avoid-ambiguous-fallthrough-in-nested-switches
description: 'When an outer switch case delegates to a nested switch where cases return, use default: break; inside and SkUNREACHABLE; after it to prevent apparent fallthrough'
rubric: readability
severity: should_fix
---

**Rule.** When an outer switch case delegates completely to an inner nested
switch whose cases all return, use `default: break;` inside the inner switch and
place `SkUNREACHABLE;` immediately after the inner switch block. Do not end the
outer case immediately on the inner switch's closing brace (such as by
terminating impossible cases with `default: SkUNREACHABLE;` inside the inner
switch), which makes the outer case look like an unintentional unannotated
fallthrough.

**Why.** Omitting any trailing statement after a nested switch creates visual
ambiguity because the outer case appears to fall through into the subsequent
case without a `[[fallthrough]];` annotation. Breaking on default inside the
inner switch and placing `SkUNREACHABLE;` directly after the inner switch block
explicitly documents that the outer case terminates while avoiding compiler
warnings about unreachable code.

## Examples

Bad:

```cpp
case Type::TypeKind::kTexture:
    switch (type.textureAccess()) {
        case Type::TextureAccess::kSample:    return "texture2d<half>";
        case Type::TextureAccess::kRead:      return "texture2d<half, access::read>";
        case Type::TextureAccess::kWrite:     return "texture2d<half, access::write>";
        case Type::TextureAccess::kReadWrite: return "texture2d<half, access::read_write>";
        default:                              SkUNREACHABLE;
    }
case Type::TypeKind::kAtomic:
```

Good:

```cpp
case Type::TypeKind::kTexture:
    switch (type.textureAccess()) {
        case Type::TextureAccess::kSample:    return "texture2d<half>";
        case Type::TextureAccess::kRead:      return "texture2d<half, access::read>";
        case Type::TextureAccess::kWrite:     return "texture2d<half, access::write>";
        case Type::TextureAccess::kReadWrite: return "texture2d<half, access::read_write>";
        default:                              break;
    }
    SkUNREACHABLE;
case Type::TypeKind::kAtomic:
```
