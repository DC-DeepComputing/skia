---
name: skia/gpu/sync-variable-names-with-refactored-types
description: When refactoring types or extracting backend structures in GPU code, update variable names to reflect the active type rather than retaining stale identifiers
rubric: readability
severity: should_fix
---

**Rule.** When refactoring or replacing types (such as migrating from
*TextureSpec to *TextureInfo), update local variable and parameter names to
reflect the replacement type rather than retaining stale identifiers from
obsolete types. Maintain consistent naming conventions across backend
implementations (such as dawnInfo, mtlInfo, and vkInfo when querying
backend-specific texture or surface info structs).

**Why.** Retaining obsolete variable names after type refactorings causes
cognitive confusion, misleads readers about the actual data structure being
inspected, and breaks naming parity across backend sibling classes.

## Examples

Bad:

```cpp
// Retaining stale 'dawnSpec' identifier after migrating from DawnTextureSpec to DawnTextureInfo:
const auto& dawnSpec = TextureInfoPriv::Get<DawnTextureInfo>(info);
wgpu::TextureFormat format = dawnSpec.getViewFormat();
```

Good:

```cpp
// Updated to 'dawnInfo' matching DawnTextureInfo and sibling vkInfo/mtlInfo conventions:
const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);
wgpu::TextureFormat format = dawnInfo.getViewFormat();
```
