---
name: skia/api/lowercase-sk-subsystem-namespaces
description: When declaring subsystem and module namespaces, use concise lowercase sk-prefixed names like skgpu or skhdr rather than PascalCase Sk-prefixed names
rubric: readability
severity: should_fix
---

**Rule.** Name subsystem and module namespaces with concise, lowercase
sk-prefixed identifiers (e.g., skgpu, sktext, skhdr, skcpu) rather than
PascalCase Sk-prefixed names. Reserve PascalCase Sk names for types (classes,
structs) and plural factory collections (e.g., SkImages, SkShaders), avoiding
name collisions and stutter with types defined inside the namespace.

**Why.** In Skia, PascalCase with an Sk prefix denotes types or static factory
collections, whereas subsystems use lowercase sk-prefixed namespaces. Using
PascalCase for subsystem namespaces creates ambiguity with class names and
forces awkward naming or stuttering for primary types inside the namespace (such
as SkHdrMetadata::Metadata instead of skhdr::Metadata).

## Examples

Bad:

```cpp
// Bad: PascalCase Sk-prefix for a subsystem namespace conflicts with type naming.
namespace SkHdrMetadata {
struct ContentLightLevelInformation { ... };
struct MasteringDisplayColorVolume { ... };
class Metadata { ... };
}
```

Good:

```cpp
// Good: Lowercase sk-prefixed subsystem namespace avoids ambiguity with enclosed types.
namespace skhdr {
struct ContentLightLevelInformation { ... };
struct MasteringDisplayColorVolume { ... };
class SK_API Metadata { ... };
}
```
