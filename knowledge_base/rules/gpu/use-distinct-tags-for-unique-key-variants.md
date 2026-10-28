---
name: skia/gpu/use-distinct-tags-for-unique-key-variants
description: When constructing UniqueKey cache keys for different resource variants or fallbacks, assign distinct and descriptive tag strings to UniqueKey::Builder
rubric: readability
severity: should_fix
---

**Rule.** When constructing UniqueKey instances for different variants,
representations, or fallback strategies of a cached GPU resource (such as
path-based versus record-ID clip masks), assign distinct, descriptive tag
strings to `skgpu::UniqueKey::Builder`. Do not reuse identical debug tag strings
across alternate keying pathways.

**Why.** UniqueKey tag strings appear in cache dumps, debugging logs, and memory
profiling tools. Reusing identical tags across different keying strategies or
fallback pathways creates ambiguity when inspecting cached entries and
diagnosing cache misses or memory footprints.

## Examples

Bad:

```cpp
// Path-keyed mask
skgpu::UniqueKey::Builder builder(&maskKey, kDomain, keySize, "Clip Path Mask");
...
// Fallback record-ID-keyed mask reuses the same tag string
skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 1, "Clip Path Mask");
```

Good:

```cpp
// Path-keyed mask
skgpu::UniqueKey::Builder builder(&maskKey, kDomain, keySize, "Clip Path Mask");
...
// Fallback record-ID-keyed mask uses a distinct descriptive tag
skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 1, "Clip SaveRecord Mask");
```
