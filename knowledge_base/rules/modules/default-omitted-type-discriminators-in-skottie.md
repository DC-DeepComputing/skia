---
name: skia/modules/default-omitted-type-discriminators-in-skottie
description: When adding a type discriminator to a parsed Skottie asset schema, default omitted fields via ParseDefault to legacy types rather than invalid sentinels
rubric: correctness
severity: should_fix
---

**Rule.** When parsing a newly introduced type discriminator property (such as
"ty") in Skottie schemas, default omitted fields via ParseDefault to the legacy
element type rather than an invalid sentinel (e.g., -1). Ensure existing assets
created before the discriminator was introduced continue to deserialize and
retain their elements under the historical default representation.

**Why.** Older Lottie or SkSL effect assets omit newly introduced discriminator
properties. Defaulting absent fields to an invalid sentinel causes the parser to
reject or drop valid legacy elements, whereas defaulting to the historical
representation preserves backward compatibility.

## Examples

Bad:

```cpp
// Bad: Omitting "ty" defaults to -1, dropping valid elements in legacy assets.
int type = ParseDefault<int>((*jprop)["ty"], -1);
if (type == kLegacyUniform) {
    // Legacy elements are dropped if "ty" is omitted in older assets.
}
```

Good:

```cpp
// Good: Defaults omitted "ty" to legacy type, maintaining backward compatibility.
int type = ParseDefault<int>((*jprop)["ty"], kLegacyUniform);
if (type == kLegacyUniform) {
    // Handles both explicitly typed and legacy untyped asset definitions.
}
```
